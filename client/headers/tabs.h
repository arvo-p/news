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

#define TAB_SIMPLE 0
#define TAB_URLBASE 1
#define TAB_TREE 2
#define TAB_GROUP 3

extern tab * tabs[16];
extern tab * selected_tab;

#define NEXT 0
#define PREVIOUS 1



