#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/nav.h"
#include "headers/main.h"
#include "headers/tabs.h"
#include "headers/ui.h"

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
	return 0;
}

int setGlobalEntry(global_e * target, entry * src){
	if(src==NULL){
		target->entry = NULL;
		target->child = NULL;
		target->parent = NULL;
		target->group_member = NULL;
		return 1;
	}
	target->entry = src;
	target->child = src->child_el;
	target->parent = target->child?target->child->parent:NULL;

	return 0;
}

int list_selector_move(int step){
	int numEntries = getEntriesNum();

	if((selected_tab->sel)+(selected_tab->line_offset)+step >= numEntries)
		return 2; // Hit higher limit

	if((selected_tab->sel)+(selected_tab->line_offset)+step < 0)
		return 1; // Hit lower limit
	
	int isWindowLarge = winSZ[0]>displayThreshold;
	int diff = isWindowLarge?(selected_tab->sel)+13+step-winSZ[1]:(selected_tab->sel)+step+5-winSZ[1];

	if(step > 0 && diff>=0){
		int actual_steps = 0;
		for(int i=0;i<step;i++){
			if(selected_tab->tab_mode == TAB_GROUP){
				g_member * gNext = getNextGroupEntry(selected_tab->offset->group_member, NEXT);
				if(gNext){
					setGlobalEntry(selected_tab->offset, gNext->entry);
					selected_tab->offset->group_member = gNext;
					actual_steps++;
				} else break;
			}else{
				entry * eNext = getNextEntry(selected_tab->offset, NEXT);
				if(eNext){
					setGlobalEntry(selected_tab->offset, eNext);
					actual_steps++;
				} else break;
			}
		}
		
		selected_tab->line_offset+=actual_steps;
		selected_tab->sel = selected_tab->selPrevious;
		return 0;
	}

	if(step < 0 && selected_tab->sel+step < 0){
		if(selected_tab->line_offset){
			int scroll_amt = -step;
			if(scroll_amt > selected_tab->line_offset) scroll_amt = selected_tab->line_offset;

			int actual_steps = 0;
			for(int i=0;i<scroll_amt;i++){
				if(selected_tab->tab_mode == TAB_GROUP){
					g_member * gPrev = getNextGroupEntry(selected_tab->offset->group_member, PREVIOUS);
					if(gPrev){
						setGlobalEntry(selected_tab->offset, gPrev->entry);
						selected_tab->offset->group_member = gPrev;
						actual_steps++;
					} else break;
				}else{
					entry * ePrev = getNextEntry(selected_tab->offset, PREVIOUS);
					if(ePrev){
						setGlobalEntry(selected_tab->offset, ePrev);
						actual_steps++;
					} else break;
				}
			}
			selected_tab->line_offset-=actual_steps;
			selected_tab->sel = selected_tab->sel + step + actual_steps;
			if(selected_tab->sel < 0) selected_tab->sel = 0;
		}else{
			return 1; //Hit lower limit
		}
		return 0;
	}

	selected_tab->sel += step;
	return 3;
}

void list_selector_goto_top(){
	selected_tab->sel = 0;
	selected_tab->line_offset = 0;
	if(selected_tab->tab_mode == TAB_SIMPLE){
		setGlobalEntry(selected_tab->offset, initial_entry);
	} else if(selected_tab->tab_mode == TAB_GROUP){
		if(selected_tab->category && selected_tab->category->first_member){
			selected_tab->offset->group_member = selected_tab->category->first_member;
			setGlobalEntry(selected_tab->offset, selected_tab->category->first_member->entry);
		}
	} else if(selected_tab->tab_mode == TAB_URLBASE){
		if(selected_tab->offset->parent && selected_tab->offset->parent->first_child){
			child_entry * first = (child_entry *)selected_tab->offset->parent->first_child;
			selected_tab->offset->child = first;
			setGlobalEntry(selected_tab->offset, first->entry);
		}
	}
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
