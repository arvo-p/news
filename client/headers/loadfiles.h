#ifndef LOADFILES_H
#define LOADFILES_H

enum LoadEntriesMode{
  	NO_OFFSET, OFFSET_LAST_ENTRY
};

struct PublicLoad;
int LOAD_INIT(struct PublicLoad * this);

#endif

