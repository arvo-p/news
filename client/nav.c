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

int setGlobalEntry(global_e * target, entry * src){
	if(src==NULL) return 1;
	target->entry = src;
	target->child = src->child_el;
	target->parent = target->child->parent;

	return 0;
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
