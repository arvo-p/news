#ifndef LOADFILES_H
#define LOADFILES_H

#include "main.h"

void get_Filepaths();
int LoadCategoryGroups();
int LoadEntries();
int loadInfoFromFile();
int updateInfoFromFile(entry * target);

#endif
