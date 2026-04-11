typedef struct colorscheme{
	int header;
	int tab_title;
	int entries;	
	int urlprefix;
	int url;
	int seen;
	int ui_walls;
	int entries_nav;
	int background;
	char urlprefix_char;
	/*
	 * Some of these configs should inherit if not set
	 */
} colorscheme;

typedef struct config{
	char name[64];
	char type;
	void * value;
	struct config * next;
} config;

extern colorscheme * colorScheme;
extern config * uiConfig;


