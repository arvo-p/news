#include "k_colorscheme.c"

#define NEXT 0
#define PREVIOUS 1

int displayThreshold = 150;

//int sel, selPrevious=0; //menu display pointer

//int tab_mode = TAB_SIMPLE;

char statusbar_notify[256];

int entries_sz=0;

int getEntriesNum();
void NewURLBASE_tab(int tab);
void draw_header();
void print_img(int width, int height, char * img);
g_member * getNextGroupEntry(g_member * gTarget, int dir);

void newline();
void EmptyScreen();
void SetCursorLastLine();

int setGlobalEntry(global_e * target, entry * src){
	if(src==NULL) return 1;
	target->entry = src;
	target->child = src->child_el;
	target->parent = target->child->parent;

	return 0;
}

void openGroup_tab(cat_group * requestedGroup){
	g_member * firstMember = requestedGroup->first_member;
	
	if(tabs_checkExist(requestedGroup->name)) return;
	selected_tab = tabs_newtab(requestedGroup->name, firstMember->entry, TAB_GROUP);
	selected_tab->offset->group_member = firstMember;
	selected_tab->category = requestedGroup;
	selected_tab->sel = 0;
	selected_tab->selPrevious = 0;

	draw_update(1);

	return;
}

void NewURLBASE_tab(int tab){
	child_entry * cE_ptr; 
	switch(tab){
		case TAB_URLBASE: 
			cE_ptr = (selected_tab->old_entry->child);

			// Prevent repeated tabs
			if(tabs_checkExist(cE_ptr->parent->url)) break;

			selected_tab = tabs_newtab(cE_ptr->parent->url, initial_entry, TAB_URLBASE);
			global_e * global_entry = selected_tab->offset;
			
			global_entry->parent = cE_ptr->parent;
			
			selected_tab->sel = 0;
			selected_tab->selPrevious = 0;	

			cE_ptr = global_entry->parent->first_child;	
			
			global_entry->child = cE_ptr;
			selected_tab->offset->entry = cE_ptr->entry;

			/* new tab append to tab array and change the tab_focus variable (?)
			 * new tab has both an id and a name(!)
			 * save the previous {offset, sel and line_offset} so user can continue reading normally, VERY IMPORTANT. Maybe create position 
			 * struct with all three elements
			 * */
			break;
		case TAB_TREE:
			/*selected_tab->tab_mode = TAB_TREE;
			
			/*
			 * h l | next and previous urlbase
			 * j k | next and previous entry


			selected_tab->line_offset = 0;
			selected_tab->sel = 0;
			selected_tab->selPrevious = 0;

			draw_update();
			*/

			break;
	}
	return;
}

int list_selector_move(int step){
	int numEntries = getEntriesNum();

	if((selected_tab->sel)+(selected_tab->line_offset)+step >= numEntries)
		return 2; // Hit higher limit
	
	int isWindowLarge = winSZ[0]>displayThreshold;
	int isWindowSmall = !isWindowLarge;
	int diff = isWindowLarge?(selected_tab->sel)+13+step-winSZ[1]:(selected_tab->sel)+step+5-winSZ[1];

	g_member * gNext = NULL;

	if(diff>=0){
		if(selected_tab->tab_mode == TAB_GROUP){
			gNext = getNextGroupEntry(selected_tab->offset->group_member, NEXT);
			setGlobalEntry(selected_tab->offset, gNext->entry);
			selected_tab->offset->group_member = gNext;
		}else{
			setGlobalEntry(selected_tab->offset,getNextEntry(selected_tab->offset, NEXT));
		}
		
		selected_tab->line_offset+=step;
		selected_tab->sel = selected_tab->selPrevious;
		return 0;
	}

	if(selected_tab->sel+step < 0){
		if(selected_tab->line_offset){
			if(selected_tab->tab_mode == TAB_GROUP){
				gNext = getNextGroupEntry(selected_tab->offset->group_member, PREVIOUS);
				setGlobalEntry(selected_tab->offset, gNext->entry);
				selected_tab->offset->group_member = gNext;
			}else{
				setGlobalEntry(selected_tab->offset, getNextEntry(selected_tab->offset, PREVIOUS));
			}
			selected_tab->line_offset+=step;
		}else{
			return 1; //Hit lower limit
		}
		selected_tab->sel = 0;
		return 0;
	}

	selected_tab->sel += step;
	return 3;
}

int getEntriesNum(){
	/*
	 * for more types of lists
	 */
	switch(selected_tab->tab_mode){
		case TAB_SIMPLE:
			return entries_sz;
			break;
		case TAB_URLBASE:
			return selected_tab->offset->parent->child_count;
			break;
		case TAB_GROUP:
			return selected_tab->category->count;
			break;
	}
}

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

g_member * getNextGroupEntry(g_member * gTarget, int dir){
	if(!gTarget){
		return NULL;
	}
	if(!dir) return !gTarget->next?NULL:gTarget->next;
	return !gTarget->previous?NULL:gTarget->previous;
}

entry * getNextEntry(global_e * requestGlobalEntry, int dir){
	entry * rTarget;
	child_entry * cTarget;
	switch(selected_tab->tab_mode){
		case TAB_SIMPLE:
			rTarget = requestGlobalEntry->entry;
			
			if(!dir) return !rTarget->next?NULL:rTarget->next;
			return rTarget->previous;
		
		case TAB_URLBASE:
			cTarget = requestGlobalEntry->child; 

			if(!dir) return !cTarget->next?NULL:cTarget->next->entry;
			return !cTarget->previous?NULL:cTarget->previous->entry;
	}
	return NULL;
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

void exit_command_mode(){
	printf("\e[%d;0H", winSZ[1]);
	printf("\e[2K");
	cmd_mode = 0;
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
