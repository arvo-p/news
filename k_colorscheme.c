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

colorscheme * colorScheme = NULL;

typedef struct config{
	char name[64];
	char type;
	void * value;
	struct config * next;
} config;

config * uiConfig;

int setAppDefaultColorscheme(){
	colorScheme->tab_title = 252;
	colorScheme->background = 10;
	
	colorScheme->entries = 250;
	colorScheme->url = 244;
	colorScheme->seen = 239;

	colorScheme->ui_walls=138;
	colorScheme->entries_nav=138;
	colorScheme->urlprefix=182;

	colorScheme->urlprefix_char = '@';

	return 0;
}

int isValidChar(int c){
	if(c > 32 && c < 127 || c == 46) return 1;
	return 0;
}

int getConfigFromLine(char * line, config * previousConfig, config * newConfig){
	//isn't this too big?

	int len = strlen(line);

	char singleArg[64];
	int singleArg_count=0;
	memset(singleArg, 0, sizeof(singleArg));
	
	int op = 0;
	/* == CODES ==
	 * op = 1 : SET
	 */

	char configName[64];
	char type;
	void * configValue;

	memset(configName, 0, sizeof(configName));
	
	int arg_num = 0;

	int validInput = 0;
	for(int i=0;i<len;i++){
		if((i == len-1 || line[i] == ';') && arg_num == 1){
			singleArg[singleArg_count] = 0;
			if(singleArg_count==0) break;

			int isNumeric = 1;
			int valueSize = strlen(singleArg);
	
			for(int j=0;j<singleArg_count;j++){
				if(singleArg[j] < 48 || singleArg[j] > 57){
					isNumeric = 0;
					break;
				}
			}

			if(isNumeric){
				configValue = (void *)malloc(sizeof(int));
				*(int *)configValue = atoi(singleArg);
				type = 'i';
			}else{
				configValue = (void *)malloc((singleArg_count)*sizeof(char));
				if(singleArg_count == 1){
					((char *)configValue)[0] = singleArg[0];
					type = 'c';
				}else{
					//i don't think this is working
					strcpy((char *)configValue, singleArg);
					type = 's';
				}
			}

			validInput = 1;
		}

		if(line[i] == '=' && arg_num == 0){
			singleArg[singleArg_count] = 0;
			
			int configName_count=0;
			for(int j=0;singleArg[j]!=0;j++){
				if(singleArg[j] == ' ') continue;
				configName[configName_count] = singleArg[j];
				configName_count++;
			}

			memset(singleArg, 0, sizeof(singleArg));
			singleArg_count = 0;
			arg_num++;
			continue;
		}
		
		if(isValidChar(line[i])){
			singleArg[singleArg_count] = line[i];
			singleArg_count++;
		}
	}

	if(validInput){
		strcpy(newConfig->name,configName);
		newConfig->value = configValue;
		newConfig->next = NULL;
		newConfig->type = type; 
		if(previousConfig) previousConfig->next = newConfig;
	}

	return validInput;
}

int getConfigValue(char * configName, void * registerValue, char expectedType){
	config * config = uiConfig;
	int ret = 0;

	while(config){
		if(strcmp(config->name, configName) == 0){
			ret = 1;
			break;
		}
		config = config->next;
	}

	if(ret){
		if(expectedType == 'i' && config->type == 'i'){
			*(int *)registerValue = *(int *)config->value;
			return 1;
		}

		if(expectedType == 'c' && config->type == 'c'){
			*(char *)registerValue = *(char *)config->value;
			return 1;
		}

		if(expectedType == 's' && config->type != 'i'){
			if(config->type == 'c') ((char *)registerValue)[0] = ((char *)config->value)[0];
			else{
				strcpy((char *)registerValue, config->value); //unsafe
			}
			return 1;
		}
	}
	return 0;
}

int loadColorschemeFromFile(char * path){
	char buffer[255];
	
	FILE * f = fopen(path, "rb");
	if(!f) return 1;

	uiConfig = malloc(sizeof(struct config));
	config * previousConfig = NULL;
	config * newConfig = uiConfig;
	
	while(fgets(buffer, 255, f)){
		int validConfig = getConfigFromLine(buffer, previousConfig, newConfig);	
		if(validConfig){
			previousConfig = newConfig;
			newConfig = malloc(sizeof(struct config));
		}
	}

	fclose(f);

	return 0;
}

int initColorscheme(){
	colorScheme = malloc(sizeof(struct colorscheme));
	setAppDefaultColorscheme();
	/*
	 * temp
	 */
	loadColorschemeFromFile("C:/Users/gsppe/wezterm/news/colorschemes/latenight");

	getConfigValue("ui.entries.color", &colorScheme->entries, 'i');
	getConfigValue("ui.url.color", &colorScheme->url, 'i');
	getConfigValue("ui.seen.color", &colorScheme->seen, 'i');
	getConfigValue("ui.walls.color", &colorScheme->ui_walls, 'i');
	getConfigValue("ui.urlprefix.color", &colorScheme->urlprefix, 'i');
	getConfigValue("ui.entries_nav.color", &colorScheme->entries_nav, 'i');
	
	getConfigValue("ui.urlprefix.char", &colorScheme->urlprefix_char, 'c');
}
