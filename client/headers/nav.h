#ifndef NAV_H
#define NAV_H

#include "main.h"

g_member * getNextGroupEntry(g_member * gTarget, int dir);
entry * getNextEntry(global_e * requestGlobalEntry, int dir);

int list_selector_move(int step);
void list_selector_goto_top();
int setGlobalEntry(global_e * target, entry * src);
int getEntriesNum();

#endif
