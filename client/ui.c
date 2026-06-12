#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdarg.h>
#include "headers/ui.h"
#include "headers/main.h"
#include "headers/tabs.h"
#include "headers/colorscheme.h"
#include "headers/nav.h"
#include "headers/cmd.h"

int displayThreshold = 150;
char statusbar_notify[256] = {0};
int entries_sz=0;

static char *ui_buffer = NULL;
static size_t ui_buffer_size = 0;
static size_t ui_buffer_pos = 0;

void ui_printf(const char *format, ...) {
    if (!ui_buffer) {
        ui_buffer_size = 1024 * 64;
        ui_buffer = malloc(ui_buffer_size);
    }
    
    va_list args;
    va_start(args, format);
    int needed = vsnprintf(ui_buffer + ui_buffer_pos, ui_buffer_size - ui_buffer_pos, format, args);
    va_end(args);

    if (needed < 0) return;

    if ((size_t)needed >= ui_buffer_size - ui_buffer_pos) {
        ui_buffer_size = (ui_buffer_pos + needed) * 2 + 1024;
        ui_buffer = realloc(ui_buffer, ui_buffer_size);
        va_start(args, format);
        vsnprintf(ui_buffer + ui_buffer_pos, ui_buffer_size - ui_buffer_pos, format, args);
        va_end(args);
    }
    
    ui_buffer_pos += needed;
}

void ui_flush() {
    if (ui_buffer && ui_buffer_pos > 0) {
        fwrite(ui_buffer, 1, ui_buffer_pos, stdout);
        fflush(stdout);
        ui_buffer_pos = 0;
    }
}

void draw_header();
void draw_command_line();
void draw_cError();
void draw_statusbar();
void draw_tabs();

void newline();
void EmptyScreen();
void SetCursorLastLine();

void * winResize_Loop(void * arg){
	int m = display_mode * (-2) + 1;
	while(1){
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		winSZ[0] = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		winSZ[1] = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;	

		if(winSZ[0] != winSZ[2] || winSZ[1] != winSZ[3]){
		
			if(winSZ[0]*m > displayThreshold*m){
				display_mode = !display_mode;
				m = display_mode * (-2) + 1;
				tabs_updateDisplayMode(selected_tab);
			}

			draw_update(true);
		}

		winSZ[2] = winSZ[0];
		winSZ[3] = winSZ[1];
		Sleep(300);
	}

	return 0;
}

void draw_header(){
	char * appname = "KOALA NEWS";
	int margin = (winSZ[0] - (int)strlen(appname))/2-8;
	// int expanded = winSZ[0] > displayThreshold; // unused
	ui_printf("\n");
	for(int i=0;i<margin/8;i++) ui_printf("\t");
	for(int i=0;i<margin%8;i++) ui_printf(" ");
	ui_printf("\e[38;5;%dm",colorScheme->tab_title);
	ui_printf("%s\e[0m\n", appname);

	return;
}

int display_urltree(){
	return 0;
}

int the_entry_print(entry * entry_item, int expanded_mode, int maxUrl, int title_maxlen, int i, short refresh){
	int shorted = 0;
	
	int len = strlen(entry_item->title);
	if(len>=title_maxlen){
		len = title_maxlen-1;
		shorted = 1;
	}

	if(expanded_mode){
		if(refresh) ui_printf("\e[38;5;%dm\e[%dG\u2502 ", colorScheme->ui_walls, 5);
		else  ui_printf("\e[%dG",7);
	}

	if(entry_item->seen) ui_printf("\e[0;38;5;%dm",colorScheme->seen);
	else ui_printf("\e[38;5;%dm", colorScheme->entries);
	
	if(i == selected_tab->sel){
		setGlobalEntry(selected_tab->old_entry, entry_item);
		ui_printf("\e[7m");
	}

	ui_printf("%.*s",len,entry_item->title);
	ui_printf("\e[0K");
	if(shorted) ui_printf("...");
		
	if(entry_item->downloaded) ui_printf(" 🌠");

	if(expanded_mode){
		ui_printf("\e[7G"); // cursor pos 7 :: title
		if(i == selected_tab->sel && shorted) ui_printf("\e[38;5;%d;7m%s \e[27m", colorScheme->entries, entry_item->title); 
		ui_printf("\e[104G"); // cursor pos 104 :: url
		ui_printf("\e[27;38;5;%dm%c ",colorScheme->urlprefix,colorScheme->urlprefix_char);
		if(i == selected_tab->sel) ui_printf("\e[4m"); // underline open
		ui_printf("\e[38;5;%dm%.*s",colorScheme->url,maxUrl,entry_item->url);
		if(i == selected_tab->sel) ui_printf("\e[24m"); // underline close
	}else{
		if(i == selected_tab->sel){
			ui_printf("\e[27;38;5;%dm", colorScheme->urlprefix);
			newline();
			ui_printf("\e[2K");
			ui_printf(" | ");
			ui_printf("\e[38;5;%dm%.*s",colorScheme->url,maxUrl,entry_item->url);
		}
	}
	newline();
	return 0;
}

int display_entries(short refresh){
	global_e * huidig = selected_tab->working; 
	if(setGlobalEntry(huidig, selected_tab->offset->entry)){
		ui_printf("\n    (Empty feed)\e[0K\n");
		return 0;
	}
	huidig->group_member = selected_tab->offset->group_member;
	
	int expanded_mode = winSZ[0]>displayThreshold;
	int maxHeight = expanded_mode?winSZ[1]-13:winSZ[1]-4; 

	int maxUrl = expanded_mode?winSZ[0]-108:winSZ[0]-3;
	int title_maxlen = expanded_mode?90:winSZ[0]-4;
	
	int i;
	for(i=0;i < maxHeight;i++){		
		the_entry_print(huidig->entry, expanded_mode, maxUrl, title_maxlen, i, refresh);

		if(selected_tab->tab_mode == TAB_GROUP){
			g_member * nextInGroup = getNextGroupEntry(huidig->group_member, NEXT);
			if(!nextInGroup) break;
			huidig->entry = nextInGroup->entry;
			huidig->group_member = nextInGroup;
			continue;
		}
		
		if(setGlobalEntry(huidig, getNextEntry(huidig, NEXT))) break;
	}

	if(expanded_mode){
		ui_printf("\e[0;2;38;5;%dm", colorScheme->ui_walls);
		ui_printf("\e[%dG\u2502\e[0K",5);
		ui_printf("\n\e[%dG\u2514\u2500\e[0K",5);
	}else{
		int margin = winSZ[0]-10;
		ui_printf("\e[%dG\e[2K",margin);
	}
	ui_printf("\e[38;5;%dm%d\\%d",colorScheme->entries_nav, (selected_tab->sel)+(selected_tab->line_offset),getEntriesNum()-1); 

	return 0;
}

void draw_tabs(){
	int len=0;
	ui_printf("\e[48;5;234m");
	int restore_bg_color = 0;

	// Write something in case draw_tabs() exceeds screen width
	
	for(len=0;tabs[len]!=0;len++){
		if(restore_bg_color){
			restore_bg_color = 0;
			ui_printf("\e[48;5;234m");
		}
		if(tabs[len] == selected_tab){
			ui_printf("\e[48;5;236m");
			restore_bg_color = 1;
		}
		ui_printf("    %s    ", tabs[len]->title);
	}
	ui_printf("\e[0m");
	newline();
	newline();
}

void draw_statusbar(){
	int expanded_mode = winSZ[0]>displayThreshold;
	int sz = (int)strlen(statusbar_notify);
	int space_sz = winSZ[0]-sz;
	char * style = "\e[48;5;233m\e[38;5;188m";
	
	SetCursorLastLine();

	if(statusbar_notify[0] != 0){
		if(expanded_mode){
			for(int i=0;i<space_sz;i++)ui_printf(" ");
			ui_printf("%s%s", style, statusbar_notify);
		}else{
			ui_printf("\r%s%.50s...", style, statusbar_notify);
		}
	}
}

void draw_cError(){
	SetCursorLastLine();
	if(cError[0]) ui_printf("\r\e[0;31m%s", cError);
	ui_flush();
}

void EmptyScreen(){
	ui_printf("\e[0m\e[H\e[2J");
}

void newline(){
	ui_printf("\n");
}

void draw_command_line(){
	ui_printf("\e[%d;0H", winSZ[1]);
	ui_printf("\e[2K");

	if(cHelper){
		ui_printf("\r");
		for(int i=0;i<cmd_count-1;i++) ui_printf(" ");
		ui_printf("\e[0;2m%s", cHelper+cHelper_len);
	}
	
	ui_printf("\r\e[0m");
	ui_printf("%s", cmd_buffer);
	ui_flush();
}

void draw_update(short refresh){	
	if(cmd_mode){
		draw_command_line();
		return;
	}
	
	if(refresh == 0){
		ui_printf("\e[0m\e[3;0H");
	}else{
		EmptyScreen();
		draw_tabs();
	}

	if(selected_tab->tab_mode == TAB_TREE) display_urltree();
	else display_entries(refresh);

	ui_flush();
	return;
}

void SetCursorLastLine(){
	ui_printf("\e[%d;0H",winSZ[1]);
}

void ClearLastLine(){
	ui_printf("\e[0m\e[%d;0H\e[2K",winSZ[1]);
	ui_flush();
}
