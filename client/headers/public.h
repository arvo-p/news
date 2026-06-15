struct entry;
typedef struct PublicLoad{
 	int (* ReloadEntries)();
	int (* UpdateInteractionInformation)(struct entry * target);
} PublicLoad;
