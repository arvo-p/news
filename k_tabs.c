/*
 * REQUIRED functions
 */

void draw_update(short refresh);

/*
* K TABS
*/

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

#define TAB_SIMPLE 0
#define TAB_URLBASE 1
#define TAB_TREE 2
#define TAB_GROUP 3

tab * tabs[16];
tab * selected_tab;

#define NEXT 0
#define PREVIOUS 1

int tabs_checkExist(char * title);

tab * tabs_newtab(char * title, entry * offset, int tab_mode){
	tab * new_tab = malloc(sizeof(tab));
	memset(new_tab, 0, sizeof(tab));

	new_tab->working = malloc(sizeof(global_e));
	new_tab->offset = malloc(sizeof(global_e));
	new_tab->old_entry = malloc(sizeof(global_e));

	setGlobalEntry(new_tab->offset, offset);
	//new_tab->offset = offset;

	new_tab->line_offset = 0;
	
	new_tab->tab_mode = tab_mode;
	strcpy(new_tab->title, title);	

	int i=0;
	while(i < sizeof(tabs) && tabs[i] != 0) i++;
	new_tab->tab_index = i;
	tabs[i] = new_tab;

	return new_tab;
}

int tabs_updateDisplayMode(tab * target){
	if(display_mode != target->display_mode){	
		target->display_mode = display_mode;
		int absolute_pos = selected_tab->line_offset+selected_tab->sel;
		
		int num_pages = 0;
		if(display_mode == 0) num_pages = (absolute_pos) / (winSZ[1]-6);
		if(display_mode == 1) num_pages = (absolute_pos) / (winSZ[1]-3);
		
		if(num_pages){
				selected_tab->line_offset = absolute_pos-1;
				selected_tab->offset->entry = getNextEntry(selected_tab->old_entry, PREVIOUS);
				selected_tab->sel = 1;
		}
		draw_update(true);
	}
	return 0;
}

int getEntriesNum();

void tabs_close(tab * target){
	int tabIndex = target->tab_index;

	if(tabIndex == 0) return;
	
	int len=0;
	for(len=0;tabs[len]!=0;len++);

	if(tabIndex == len-1){
		tabs[tabIndex] = NULL;
		if(tabIndex > 1) selected_tab = tabs[--tabIndex];
		else selected_tab = tabs[0];
	}else{
		for(int i=tabIndex;i<len-1;i++){
			tabs[i] = tabs[i+1];
			tabs[i]->tab_index--;
		}
		tabs[len-1] = 0;
		if(target == selected_tab) selected_tab = tabs[tabIndex];	
	}

	free(target);
	return;
}

int tabs_checkExist(char * title){
	int sz = strlen(title);
	
	for(int i=0;tabs[i]!=0;i++){
		if(strncmp(tabs[i]->title,title,sz)==0) return 1;
	}
	
	return 0;
}

void tabs_switchNext(tab * target){
	int len=0;
	for(len=0;tabs[len]!=0;len++);
	int new_index = (target->tab_index+1)%(len);
	selected_tab = tabs[new_index];	

	/*
	 * fix jumping selection line
	 */
	tabs_updateDisplayMode(selected_tab);
	
	return;
}


