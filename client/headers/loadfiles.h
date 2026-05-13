#ifndef LOADFILES_H
#define LOADFILES_H

#include "main.h"

void get_Filepaths();
int LoadCategoryGroups();

enum LoadEntriesMode{
  	NO_OFFSET, OFFSET_LAST_ENTRY
};

int LoadEntries(enum LoadEntriesMode mode);
int loadInfoFromFile();
int updateInfoFromFile(entry * target);

#endif
