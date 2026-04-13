#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <conio.h>
#include <windows.h>
#include <shellapi.h>
#include <direct.h>
#include <io.h>
#include <time.h>

#include "headers/main.h"
#include "headers/nav.h"
#include "headers/ui.h"
#include "headers/cmd.h"
#include "headers/tabs.h"
#include "headers/colorscheme.h"
#include "headers/loadfiles.h"

cat_group * initial_group = NULL;
entry * initial_entry = NULL;
entry_parent * initial_parent = NULL;

int winSZ[4] = {0,0,0,0};
int display_mode = 1;

int entry_view(entry * entry);

int main(int argc, char * argv[]){
	
	pthread_t tWinResize;
	
	int highlighted_entry_id = -1; 
	if(argc == 2) highlighted_entry_id = atoi(argv[1]);

	get_Filepaths();
	LoadCategoryGroups();
	LoadEntries();
	loadInfoFromFile();	
	initColorscheme();
	
	tabs[0] = tabs_newtab("Main", initial_entry, TAB_SIMPLE);
	selected_tab = tabs[0];

	SetConsoleOutputCP(CP_UTF8);
	srand(time(NULL));  
	
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	winSZ[0] = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	winSZ[1] = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	winSZ[2] = winSZ[0];
	winSZ[3] = winSZ[1];

	if (winSZ[0] > displayThreshold) display_mode = 0;
	else display_mode = 1;
	
	draw_update(true);

	pthread_create(&tWinResize, NULL, &winResize_Loop, NULL);
	while(!winSZ[0]); //wait for winresize

	int input;
	int scroll_ret = 0;
	int redraw = false;

	printf("\e[?25l"); //invisible cursor
	while(1){
		redraw = false;
		input = getch();
		if(_kbhit() != 0) continue; // Prevent input spamming
		
		selected_tab->selPrevious = selected_tab->sel;
		
		if(cError[0] || statusbar_notify[0]){
			statusbar_notify[0] = 0;
			cError[0] = 0;
			ClearLastLine();
		}
	
		if(cmd_mode){
			switch(input){
				case 13:
					exit_command_mode();
					cHelper = NULL;
					if(run_cmd(0)!=0) draw_cError();
					memset(cmd_buffer, 0, cmd_count);
					cmd_count = 0;
					
					break;
				case 8:
					if(cmd_count>0) cmd_buffer[--cmd_count] = 0;
					if(cmd_count == 0) exit_command_mode();
					if(!cHelper) run_cmd(1);
					else if(cHelper_len) cHelper_len--;
					else cHelper = NULL;
					
					break;
				case 9:
					cmd_autocomplete();
					break;
				default:
					if(cmd_count < 100) cmd_buffer[cmd_count++] = input;
					run_cmd(1);
					break;
			}
			if(!cmd_mode) printf("\e[?25h"); //visible cursor
			
			draw_update(false);	
			
			continue;
		}

		switch(input){
			case 'T': 
				//save local copy
				break;
			case 13:
				entry_view(selected_tab->old_entry->entry);
				break;
			case 'j':
				scroll_ret = list_selector_move(1);
				if(scroll_ret == 2) continue;
				break;
			case 'k':
				scroll_ret = list_selector_move(-1);
				if(scroll_ret == 1) continue;
				break;
			case 'u':
				tabs_NewURLBASE(TAB_URLBASE);
				redraw = true;
				break;
			case 'n':
				tabs_switchNext(selected_tab);
				redraw = true;
				break;
			case 'c':
				tabs_close(selected_tab);
				redraw = true;
				break;
			case ':':
				cmd_buffer[cmd_count++] = ':';
				cmd_mode = 1;
				printf("\e[?25h");
				break;
			case 'Q':
				printf("\e[0m\e[?25h");
				exit(0);
		}
		draw_update(redraw);
	}

	return 0;
}

int entry_view(entry * entry){
	char * url = entry->url;
	short szUrl = strlen(url);
	entry->seen = 1;

	char cmd[1024];
	DWORD creationFlags = DETACHED_PROCESS;

	if(strstr(url, "youtube.com") || strstr(url, "youtu.be") || strstr(url+szUrl-4,".mp4")){
		snprintf(cmd, sizeof(cmd), "cmd.exe /c mpv --force-window=immediate  --osd-level=1 \"%s\"", url);
		creationFlags = CREATE_NEW_CONSOLE;
	}else{
		snprintf(cmd, sizeof(cmd), "cmd.exe /c start \"\" \"%s\"", url);
		creationFlags = DETACHED_PROCESS;
	}

	STARTUPINFOA si = { sizeof(si) };
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	if (CreateProcessA(NULL, cmd, NULL, NULL, FALSE, creationFlags, NULL, NULL, &si, &pi)) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	updateInfoFromFile(entry); //mark as seen

	return 0;
}
