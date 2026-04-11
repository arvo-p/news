#ifndef UI_H
#define UI_H

#include "main.h"

#define NEXT 0
#define PREVIOUS 1

int getEntriesNum();
int list_selector_move(int step);

void draw_update(short refresh);
void draw_cError();
void ClearLastLine();
void * winResize_Loop(void * arg);

extern int displayThreshold;
extern char statusbar_notify[256];
extern int entries_sz;

#endif
