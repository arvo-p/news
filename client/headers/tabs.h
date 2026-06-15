#ifndef TABS_H
#define TABS_H

#include "main.h"

typedef struct tab{
	char title[128];
	global_e * working;
	global_e * offset;
	global_e * old_entry;
	
	/* === Experimental ===
		chunk * loaded_chunk;
		int index_chunk;
	*/

	cat_group * category;
	int tab_mode;
	int sel;
	int selPrevious;
	int line_offset;
	int display_mode;
	int tab_index;
} tab;

int tabs_checkExist(char * title);
tab * tabs_newtab(char * title, entry * offset, int tab_mode);
void tabs_close(tab * target);
void tabs_switchNext(tab * target);
void tabs_switchPrevious(tab * target);
void tabs_openGroup(cat_group * requestedGroup);
void tabs_NewURLBASE(int tab);
int tabs_updateDisplayMode(tab * target);

#define TAB_SIMPLE 0
#define TAB_URLBASE 1
#define TAB_TREE 2
#define TAB_GROUP 3

extern tab * tabs[16];
extern tab * selected_tab;

#define NEXT 0
#define PREVIOUS 1

#endif
