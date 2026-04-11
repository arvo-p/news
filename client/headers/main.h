#ifndef MAIN_H
#define MAIN_H

#define TAB_URLBASE 1
#define TAB_TREE 2
#define TAB_GROUP 3

typedef struct entry_parent{
	unsigned long id;
	char url[70];
	struct entry_parent * next;
	unsigned int child_count;
	void * first_child;
} entry_parent;

typedef struct cat_group{
	unsigned long id;
	char name[21];
	struct cat_group * next;
	struct g_member * first_member;
	int count;
} cat_group;

typedef struct g_member{
	struct entry * entry;
	struct g_member * next;
	struct g_member * previous;
} g_member;

typedef struct entry{
	unsigned long id;
	char title[320];
	char url[220];
	char * url_nohttp;
	unsigned char seen:1;
	unsigned char downloaded:1;
	struct entry * next;
	struct entry * previous;
	void * child_el;
} entry;

typedef struct child_entry{
	unsigned int id;
	struct entry_parent * parent;
	struct entry * entry;
	struct child_entry * next;
	unsigned int position;
	struct child_entry * previous;
} child_entry;

typedef struct global_e{
	entry * entry;
	child_entry * child;
	entry_parent * parent;
	g_member * group_member;
	cat_group * category;
} global_e;

typedef struct chunk{
	entry entry[32];
	int start_index;
	struct chunk * next;
	struct chunk * previous;
} chunk;

typedef struct pos{
	chunk * chunk;
	int index;
	entry * optional_entry;
} pos; 

entry * getNextEntry(global_e * requestGlobalEntry, int dir);
int setGlobalEntry(global_e * target, entry * src);

// --- Global Variables (Extern) ---

extern cat_group * initial_group;
extern entry * initial_entry;
extern entry_parent * initial_parent;

extern int winSZ[4];
extern int display_mode;

#endif
