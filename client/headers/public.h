struct entry;
struct cat_group;

typedef struct {
	char url[256];
	char alias[128];
	struct cat_group * groups[20];
	int group_count;
} webfeed_info;

extern webfeed_info webfeeds_list[200];
extern int webfeeds_count;

typedef struct PublicLoad{
 	int (* ReloadEntries)();
	int (* UpdateInteractionInformation)(struct entry * target);

	int (* ModifyGroups)(int groupid, const char * groupname, short action);
	int (* ModifyWebfeeds)(const char * target_url, const char * old_groupname, const char * new_groupname, short action);
} PublicLoad;
