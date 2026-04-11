/*
* K TABS
*/
tab * tabs[16];
tab * selected_tab;

tab * tabs_newtab(char * title, entry * offset, int tab_mode){
	tab * new_tab = malloc(sizeof(tab));
	memset(new_tab, 0, sizeof(tab));

	new_tab->working = malloc(sizeof(global_e));
	new_tab->offset = malloc(sizeof(global_e));
	new_tab->old_entry = malloc(sizeof(global_e));

	setGlobalEntry(new_tab->offset, offset);

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
	}
	return 0;
}

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
	for(int i=0;tabs[i]!=0;i++) if(strncmp(tabs[i]->title,title,sz)==0) return 1;
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

void tabs_openGroup(cat_group * requestedGroup){
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

void tabs_NewURLBASE(int tab){
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
