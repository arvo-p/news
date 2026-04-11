int displayThreshold = 150;
char statusbar_notify[256];
int entries_sz=0;

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
				display_mode != display_mode;
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
	int margin = (winSZ[0] - strlen(appname))/2-8;
	int expanded = winSZ[0] > displayThreshold;
	printf("\n");
	for(int i=0;i<margin/8;i++) printf("\t");
	for(int i=0;i<margin%8;i++) printf(" ");
	printf("\e[38;5;%dm",colorScheme->tab_title);
	printf("%s\e[0m\n", appname);

	return;
}

int display_urltree(){
	return 0;
}

int the_entry_print(entry * entry, int expanded_mode, int maxUrl, int title_maxlen, int i, short refresh){
	int shorted = 0;
	char * url;
	
	int len = strlen(entry->title);
	if(len>=title_maxlen){
		len = title_maxlen-1;
		shorted = 1;
	}

	if(expanded_mode){
		if(refresh) printf("\e[38;5;%dm    \u2502 ", colorScheme->ui_walls);
		else  printf("\e[%dG",7);
	}

	if(entry->seen) printf("\e[0;38;5;%dm",colorScheme->seen);
	else printf("\e[38;5;%dm", colorScheme->entries);
	
	if(i == selected_tab->sel){
		setGlobalEntry(selected_tab->old_entry, entry);
		printf("\e[7m");
	}

	printf("%.*s",len,entry->title);
	printf("\e[0K");
	if(shorted) printf("...");
		
	if(entry->downloaded) printf(" 🌠");

	if(expanded_mode){
		printf("\e[7G"); // cursor pos 7 :: title
		if(i == selected_tab->sel && shorted) printf("\e[38;5;%d;7m%s \e[27m", colorScheme->entries, entry->title); 
		printf("\e[104G"); // cursor pos 104 :: url
		printf("\e[27;38;5;%dm%c ",colorScheme->urlprefix,colorScheme->urlprefix_char);
		if(i == selected_tab->sel) printf("\e[4m"); // underline open
		printf("\e[38;5;%dm%.*s",colorScheme->url,maxUrl,entry->url);
		if(i == selected_tab->sel) printf("\e[24m"); // underline close
	}else{
		if(i == selected_tab->sel){
			printf("\e[27;38;5;%dm", colorScheme->urlprefix);
			newline();
			printf("\e[2K");
			printf(" | ");
			printf("\e[38;5;%dm%.*s",colorScheme->url,maxUrl,entry->url);
		}
	}
	newline();
}

int display_entries(short refresh){

	global_e * huidig = selected_tab->working; 
	setGlobalEntry(huidig, selected_tab->offset->entry);
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
		printf("\e[0;2;38;5;%dm", colorScheme->ui_walls);
		printf("    \u2502\e[0K");
		printf("\n    \u2514\u2500\e[0K");
	}else{
		int margin = winSZ[0]-10;
		printf("\e[%dG\e[2K",margin);
	}
	printf("\e[38;5;%dm%d\\%d",colorScheme->entries_nav, (selected_tab->sel)+(selected_tab->line_offset),getEntriesNum()-1); 

	return 0;
}

void draw_tabs(){
	int len=0;
	printf("\e[48;5;234m");
	int restore_bg_color = 0;

	// Write something in case draw_tabs() exceeds screen width
	
	for(len=0;tabs[len]!=0;len++){
		if(restore_bg_color){
			restore_bg_color = 0;
			printf("\e[48;5;234m");
		}
		if(tabs[len] == selected_tab){
			printf("\e[48;5;236m");
			restore_bg_color = 1;
		}
		printf("    %s    ", tabs[len]->title);
	}
	printf("\e[0m");
	newline();
	newline();
}

void draw_statusbar(){
	int expanded_mode = winSZ[0]>displayThreshold;
	int sz = strlen(statusbar_notify);
	int space_sz = winSZ[0]-sz;
	char * style = "\e[48;5;233m\e[38;5;188m";
	
	SetCursorLastLine();

	if(statusbar_notify[0] != 0){
		if(expanded_mode){
			for(int i=0;i<space_sz;i++)printf(" ");
			printf("%s%s", style, statusbar_notify);
		}else{
			printf("\r%s%.50s...", style, statusbar_notify);
		}
	}
}

void draw_cError(){
	SetCursorLastLine();
	if(cError[0]) printf("\r\e[0;31m%s", cError);
}

void EmptyScreen(){
	printf("\e[0m\e[H\e[2J");
}

void newline(){
	printf("\n");
}

void draw_command_line(){
	printf("\e[%d;0H", winSZ[1]);
	printf("\e[2K");

	if(cHelper){
		printf("\r");
		for(int i=0;i<cmd_count-1;i++) printf(" ");
		printf("\e[0;2m%s", cHelper+cHelper_len);
	}
	
	printf("\r\e[0m");
	printf("%s", cmd_buffer);

}

void draw_update(short refresh){	
	int expanded_mode = winSZ[0]>displayThreshold;
	
	if(cmd_mode){
		draw_command_line();
		return;
	}
	
	if(refresh == 0){
		printf("\e[0m\e[3;0H");
	}else{
		EmptyScreen();
		draw_tabs();
	}

	if(selected_tab->tab_mode == TAB_TREE) display_urltree();
	else display_entries(refresh);

	return;
}

void SetCursorLastLine(){
	printf("\e[%d;0H",winSZ[1]);
}

void ClearLastLine(){
	printf("\e[0m\e[%d;0H\e[2K",winSZ[1]);
}
