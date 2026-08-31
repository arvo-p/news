#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <shlobj.h>
#include <direct.h>
#include "headers/loadfiles.h"
#include "headers/tabs.h"
#include "headers/main.h"
#include "headers/ui.h"

char *pth_gConf, *pth_folder_colorscheme, *pth_Root, * pth_EntriesInfo, * pth_EntriesInfoMod, * pth_mainNews, * pth_Archive, *pth_Webfeeds;

int LOAD_GetEntries(enum LoadEntriesMode mode);
int LOAD_GetInteractionInformation();
int LOAD_UpdateInteractionInformation(entry * target);
int LOAD_GetFilepaths();
int LOAD_GetCategoryGroups();
int LOAD_ReloadEntries();
int LOAD_ModifyGroups(int groupid, const char * groupname, short action);
int LOAD_ModifyWebfeeds(const char * target_url, const char * old_groupname, const char * new_groupname, short action);
int LOAD_GetWebfeedsList();

short homepageExcludedGroups[32];

webfeed_info webfeeds_list[200];
int webfeeds_count = 0;

int LOAD_INIT(struct PublicLoad * this){
	int error = 0;

	error = LOAD_GetFilepaths();
	error += LOAD_GetCategoryGroups() << 1;
	error += LOAD_GetWebfeedsList() << 3;
	error += LOAD_GetEntries(NO_OFFSET) << 2;
	error += LOAD_GetInteractionInformation() << 4;	

	if(error != 0){
		printf("LOAD_INIT ERROR %d", error);
     	return 1;
	}

	this->ReloadEntries = &LOAD_ReloadEntries;
	this->UpdateInteractionInformation = &LOAD_UpdateInteractionInformation;
	this->ModifyGroups = &LOAD_ModifyGroups;
	this->ModifyWebfeeds = &LOAD_ModifyWebfeeds;

	return 0;
}

int LOAD_ReloadEntries(){
	LOAD_GetEntries(OFFSET_LAST_ENTRY);
	if(selected_tab->tab_mode == TAB_SIMPLE) setGlobalEntry(selected_tab->offset, initial_entry);
	else if(selected_tab->tab_mode == TAB_GROUP){
		selected_tab->offset->group_member = selected_tab->category->first_member;
		setGlobalEntry(selected_tab->offset, selected_tab->category->first_member->entry);
	}
	return 0;
}

char * pathAppend(char * pth, char * toAppend){
	int len = strlen(pth);
	int addDir = 0;
	if(pth[len-1] != '\\') addDir=1;

	char * newstr = malloc(len+strlen(toAppend)+addDir+1);
	
	snprintf(newstr, len + strlen(toAppend) + 2, "%s%s%s", pth, addDir ? "\\" : "", toAppend);
	return newstr;
}

int GetLineParameter(short positionParameter, char * src, short sz, char * dest){
	short countPosition = 0;
	int offset=-1;
	int szSub=0;
	int i=0;

	for(i=0;i<sz;i++){
		if(offset != -1){
			szSub++;
			if(src[i] == 31) break;
		}
		if(szSub == 0){
			if(src[i] == 31) countPosition++;
 			if(countPosition == positionParameter)
				offset = i;
		}
	}
	
	if(offset == -1 || szSub == 0) return 1;
	strncpy(dest, src+offset, szSub);
	dest[szSub] = 0;

	return 0;
}

int LOAD_GetFilepaths(){
	char * file = "int.dat";
	char * file_m = "in_.dat";
	char * file_gConf = "user_groups.conf";
	char * file_news = "entries.dat";
	char * folder_archive = "archive\\";
	char * folder_colorscheme = "colorscheme\\";

	char pth_Appdata[MAX_PATH];

	SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, pth_Appdata);
	pth_Root = pathAppend(pth_Appdata, "kNews");
	_mkdir(pth_Root);

	pth_EntriesInfo = pathAppend(pth_Root, file);
	pth_EntriesInfoMod = pathAppend(pth_Root, file_m);
	pth_mainNews = pathAppend(pth_Root, file_news);
	pth_gConf = pathAppend(pth_Root, file_gConf);
	pth_Webfeeds = pathAppend(pth_Root, "webfeeds.conf");
	pth_Archive = pathAppend(pth_Root, folder_archive);
	pth_folder_colorscheme = pathAppend(pth_Root, folder_colorscheme);

	return 0;
}

int LOAD_GetWebfeedsList(){
	webfeeds_count = 0;
	FILE *fin = fopen(pth_Webfeeds, "rb");
	if (!fin){
		fin = fopen(pth_Webfeeds, "wb");
		if (fin) fclose(fin);
		return 0;
	}

	char line[512];
	while(fgets(line, sizeof(line), fin) && webfeeds_count < 200){
		size_t len = strlen(line);
		while(len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')){
			line[len-1] = 0;
			len--;
		}
		if (len == 0) continue;
		
		char url[256] = {0};
		char groups_str[256] = {0};
		char alias_str[128] = {0};

		char *first_space = strchr(line, ' ');
		if (first_space){
			int u_len = first_space - line;
			strncpy(url, line, u_len);
			url[u_len] = 0;
			
			char *metadata = first_space;
			
			char *bracket_start = strchr(metadata, '[');
			if (bracket_start){
				char *bracket_end = strchr(bracket_start, ']');
				if (bracket_end){
					int g_len = bracket_end - bracket_start - 1;
					strncpy(groups_str, bracket_start + 1, g_len);
					groups_str[g_len] = 0;
				}
			}
			
			char *paren_start = strchr(metadata, '(');
			if (paren_start){
				char *paren_end = strchr(paren_start, ')');
				if (paren_end){
					int a_len = paren_end - paren_start - 1;
					strncpy(alias_str, paren_start + 1, a_len);
					alias_str[a_len] = 0;
				}
			}
		} else {
			strcpy(url, line);
		}
		
		if (strlen(url) > 0){
			char *start = url;
			if (strncmp(start, "https://", 8) == 0) start += 8;
			else if (strncmp(start, "http://", 7) == 0) start += 7;
			
			if (strncmp(start, "www.", 4) == 0) start += 4;

			strcpy(webfeeds_list[webfeeds_count].url, start);
			strcpy(webfeeds_list[webfeeds_count].alias, alias_str);
			webfeeds_list[webfeeds_count].group_count = 0;
			
			if (strlen(groups_str) > 0){
				char *token = strtok(groups_str, ",");
				while (token != NULL){
					while(*token == ' ') token++;
					int tlen = strlen(token);
					while(tlen > 0 && token[tlen-1] == ' '){ token[tlen-1] = 0; tlen--; }
					
					if (tlen > 0 && webfeeds_list[webfeeds_count].group_count < 20){
						cat_group * curr = initial_group;
						while(curr){
							if (_stricmp(curr->name, token) == 0){
								webfeeds_list[webfeeds_count].groups[webfeeds_list[webfeeds_count].group_count++] = curr;
								break;
							}
							curr = curr->next;
						}
					}
					token = strtok(NULL, ",");
				}
			}
			
			webfeeds_count++;
		}
	}
	fclose(fin);
	return 0;
}

int LOAD_GetCategoryGroups(){
	FILE * fp;	
	char line[320];

	cat_group * first = NULL;
	cat_group * previous = NULL;

	fp = fopen(pth_gConf, "rb");
	if(!fp){
		fp = fopen(pth_gConf, "wb");
		if (fp) fclose(fp);
		return 0;
	}
	while (fgets(line, sizeof(line), fp)){			
		if(strncmp(line, "homepage-exclude", 16) == 0){
			int step = 0;
			short last_offset = 17;
			int sz = strlen(line);
			for(int i=17;i<sz;i++){
				if(line[i] == ',' || line[i] == '\n' || i + 1 == sz){
					if(i+1 != sz) line[i] = 0;
					for(int j=0;j<32;j++){
						if(homepageExcludedGroups[j] == 0){
							homepageExcludedGroups[j] = strtoul(line+last_offset, NULL, 10);
							break;
						}
					}
					last_offset = i+1;
				}
			}
		}
		if(strncmp(line, "register-group", 14) == 0){
			int sz = strlen(line);
			int group_id = -1;
			int name_pos = -1;
			int name_length = 0;
			int step = 0;
			for(int i=15;i<sz;i++){
				if(step == 1){
					if(line[i] == ' ' || line[i] == '\n'){
						line[i] = 0;
						break;
					}
					name_length++;
				}
				if(step == 0){
					if(line[i] == ' '){
						line[i] = 0;
						step++;
						group_id = atoi(line+15);
						name_pos = i+1;
						continue;
					}
					if(!isdigit(line[i])) break;
				}
			}

			if(group_id < 0) printf("register-group : numeric value expected, in first position, for ID. Ignoring.\n");
			else if(name_length > 20) printf("register-group: group name too long (>20). Ignoring.\n");
			else{
				
				cat_group * new_group = malloc(sizeof(struct cat_group));
				new_group->next = NULL;
				new_group->id = group_id;
				new_group->first_member = NULL;
				new_group->count = 0;

				line[name_pos+name_length] = 0;
				strcpy(new_group->name, line+name_pos);
				
				if(!previous)
					first = new_group;
				else
					previous->next = new_group;
				
				previous = new_group;
			}
		}
	}

	initial_group = first;

	fclose(fp);
	return 0;
}

int LOAD_GetEntry(char * line, entry * new_entry, entry ** previous_entry, int * n_parent){
	entry_parent * new_parent = NULL;

	//Find delimiter
	char * args[5] = {NULL, NULL, NULL, NULL, NULL}; 
	int args_c = 1;

	args[0] = &line[0];
	int j;
	for(j=1;line[j];j++){
		if(line[j-1]==31){
			args[args_c]=&line[j];
			line[j-1] = 0;
			args_c++;
		}
		if(line[j] == '\n' || line[j] == 0xd){
			line[j] = 0;
		}
	}
	if(args_c < 3) return 1;

	memset(new_entry->url, 0, 220);

	new_entry->next = NULL;
	new_entry->previous = NULL;

	// Retrieve url
	int szT = strlen(args[2])+1;
	if(szT>210) szT = 210;
	strncpy(new_entry->url, args[2], szT);
	new_entry->url[szT-1] = 0;

	// Works
	if(strstr(new_entry->url, "https://"))
		new_entry->url_nohttp=new_entry->url+8;
	else if(strstr(new_entry->url, "http://"))
		new_entry->url_nohttp=new_entry->url+7;
	else
		new_entry->url_nohttp=new_entry->url;
	
	//Get urlbase and entry_parent
	unsigned int pos = 0;
	int szLimit = 69;
	for(pos=0;new_entry->url_nohttp[pos]!=0&&pos<szLimit;pos++) if(new_entry->url_nohttp[pos] == '/') break;

	entry_parent * eP_ptr = initial_parent;
	
	int createNewParent = 1;

	for(int i=0;eP_ptr;i++){
		if(strncmp(eP_ptr->url, new_entry->url_nohttp,pos)==0){
			createNewParent = 0;
			break;
		}
		eP_ptr = eP_ptr->next;
	}

	child_entry * new_child = NULL;
	new_child = malloc(sizeof(struct child_entry));
	new_child->entry = new_entry;
	new_child->next = NULL;

	if(createNewParent){	
		new_parent = malloc(sizeof(struct entry_parent));
		new_parent->id = ((*n_parent)++);
		new_parent->next = NULL;

		for(int j=0;j<pos;j++) new_parent->url[j] = new_entry->url_nohttp[j];
		new_parent->url[pos] = 0;

		if(initial_parent != NULL)
			new_parent->next = initial_parent;

		new_parent->first_child = (void *)new_child; 
		new_parent->child_count = 0;

		initial_parent = new_parent;
		eP_ptr = new_parent;
	}else{
	
		child_entry * cE_ptr = (child_entry *) eP_ptr->first_child;
		cE_ptr->previous = new_child;
		new_child->next = cE_ptr;
		eP_ptr->first_child = new_child;
	}

	new_child->id = eP_ptr->child_count++;
	new_child->parent = eP_ptr;

	//Set entry's child_entry element
	new_entry->child_el = (void *)new_child;

	// Retrieve title
	szT = strlen(args[1]);
	if(szT > 320) szT = 320; 
	strncpy(new_entry->title, args[1], szT);
	new_entry->title[szT] = 0;

	// Retrieve groups
	
	int groups[5];
	char n[12];
	memset(groups, 0, sizeof(groups));
	int g_count = 0, n_count = 0;

	int sz = strlen(args[3]);

	for(int k=0;k<sz;k++){
		if(isdigit(*(args[3]+k)) && n_count < sizeof(n)-1) n[n_count++] = *(args[3]+k);

		if(*(args[3]+k) == ' ' || n_count >= sizeof(n)-1){
			if(n_count > 0 && g_count < 5){
				n[n_count] = 0;
				groups[g_count++] = atoi(n);
			}
			n_count = 0;
		}
	}
	if(n_count > 0 && g_count < 5){
		n[n_count] = 0;
		groups[g_count++] = atoi(n);
	}

	g_member * newGroupMember = NULL, * findGroupMember = NULL;

	/*printf("\n%s belong to groups: ", new_entry->title);
	for(int i=0;groups[i];i++) printf("%d ", groups[i]);*/

	cat_group * findGroup = initial_group;
	while(findGroup){
		for(int kj=0;kj<g_count;kj++){ //matched group
			if(findGroup->id != groups[kj]) continue;
			newGroupMember = malloc(sizeof(struct g_member));
			newGroupMember->entry = new_entry;
			newGroupMember->next = NULL;
			newGroupMember->previous = NULL;

			if(!findGroup->first_member) findGroup->first_member = newGroupMember;
			else{
				findGroupMember = findGroup->first_member;
				findGroupMember->previous = newGroupMember;
				newGroupMember->next = findGroupMember;
				findGroup->first_member = newGroupMember;
			}
			findGroup->count++;
		}
		findGroup = findGroup->next;
	}

	// Set other parameters, will be retrieved later
	new_entry->seen = 0;
	new_entry->downloaded = 0;

	//new_entry->next=NULL;
	new_entry->previous=NULL;
	new_entry->id = strtoul(args[0], NULL, 16);

	short doIncludeEntryInMainFeed = 1;
	for(int i=0;i<g_count;i++){
		for(int j=0;j<32;j++){
			if(homepageExcludedGroups[j] == 0) continue;
			if(groups[i] == homepageExcludedGroups[j]){
				doIncludeEntryInMainFeed = 0;
				break;
			}
		}
		if(!doIncludeEntryInMainFeed) break;
	}
	
	if(doIncludeEntryInMainFeed){
		if(new_entry&&(*previous_entry)){
			new_entry->next = *previous_entry;
			(*previous_entry)->previous = new_entry;
		}
		*previous_entry = new_entry;
		entries_sz++;
	}	
	return 0;
}


int LOAD_GetEntries(enum LoadEntriesMode mode){
	FILE * fp;
	char line[400];

	/*
	 * Entries are loaded upside down in memory.
	 * Because the server appends new entries at the end of the file.
	 * --------------------------------------------------------------
	 * 	NO_OFFSET -> Load all entries.
	 *	OFFSET_LAST_ENTRY -> Load entries after an offset ID.
	 */

	int isOffsetConditionSatisfied = 0;
	static char * entryLastLoadedID = NULL;
	static char * offsetID = NULL;
	if(entryLastLoadedID == NULL) entryLastLoadedID = malloc(8);
	if(offsetID == NULL) offsetID = malloc(8);
	if(mode == OFFSET_LAST_ENTRY){
		strcpy(offsetID, entryLastLoadedID);
	}

	entry * previous_entry = (mode == OFFSET_LAST_ENTRY) ? initial_entry : NULL;
	static int n_parent = 0;
	static chunk * entry_block = NULL;
	
	fp = fopen(pth_mainNews, "rb");
	if(!fp){
		fp = fopen(pth_mainNews, "wb");
		if (fp) fclose(fp);
		return 0;
	}

	int i=0;
	while (fgets(line, sizeof(line), fp)){				
		GetLineParameter(0, line, strlen(line), entryLastLoadedID);

		if(mode == OFFSET_LAST_ENTRY){
			if(strcmp(offsetID, entryLastLoadedID)==0){
				isOffsetConditionSatisfied = 1;
				continue;
			} else if(!isOffsetConditionSatisfied) continue;
		}

		if(i % 96 == 0){
			chunk * new_entry_block = malloc(sizeof(struct chunk));
			memset(new_entry_block, 0, sizeof(struct chunk));
			if(entry_block != NULL){
				new_entry_block->next = entry_block;
				entry_block->previous = new_entry_block;
			}
			entry_block = new_entry_block;
			entry_block->start_index = 0;
		}

		LOAD_GetEntry(line, &entry_block->entry[95-(i%96)], &previous_entry, &n_parent);
		i++;
	}

	if(i > 0 && entry_block) entry_block->start_index = 95-((i-1)%96);
	initial_entry = previous_entry;	

	fclose(fp);

	return 0;
}

int LOAD_UpdateInteractionInformation(entry * target){
	FILE * fpr, * fpw;
	char line[320];

	fpr = fopen(pth_EntriesInfo, "rb");
	fpw = fopen(pth_EntriesInfoMod, "wb");
	if(!fpw) return 1;

	/*
	 * [ID];[SEEN];[DOWNLOADED];[BOOKMARKED]
	 */
	fprintf(fpw, "%lx;%d;%d\n", target->id, (int)target->seen, (int)target->downloaded); 
	if(!fpr) goto updateInfoFromFile_end;

	char * args[3] = {NULL, NULL, NULL};
	int args_c = 1;

	while(fgets(line, sizeof(line), fpr)){				
		memset(args, 0, sizeof args);
		args_c = 1;

		args[0] = &line[0];
		for(int j=1;line[j];j++){
			if(line[j-1]==';'){ //31
				args[args_c]=&line[j];
				line[j-1] = 0;
				args_c++;
			}
			if(args_c > 3) break;
		}
		
		if(target->id != strtoul(args[0], NULL, 16)) fprintf(fpw, "%s;%s;%s", args[0], args[1], args[2]);	
	}

	fclose(fpr);

	updateInfoFromFile_end:
	fclose(fpw);

	remove(pth_EntriesInfo);
	rename(pth_EntriesInfoMod, pth_EntriesInfo);
	return 0;
}

int LOAD_GetInteractionInformation(){
	FILE * fp;
	char line[320];

	fp = fopen(pth_EntriesInfo, "rb");
	if(!fp){
		fp = fopen(pth_EntriesInfo, "wb");
		if (fp) fclose(fp);
		return 0;
	}

	char * args[3] = {NULL, NULL, NULL};
	int args_c = 1;

	while (fgets(line, sizeof(line), fp)){				
		entry * entry_load = NULL;

		memset(args, 0, sizeof args);
		args_c = 1;

		args[0] = &line[0];
		for(int j=1;line[j];j++){
			if(line[j-1]==';'){ //31
				args[args_c]=&line[j];
				line[j-1] = 0;
				args_c++;
			}
			if(args_c > 3) break;
		}
		entry_load = initial_entry;
		while(entry_load){
			if(entry_load->id == strtoul(args[0], NULL, 16)){ //if it doesnt match -> delete from list, create a function to remove lines
									 /* save in a ToDelete array
									  * later a function will run through the list and recopy only what is outside the array
									  * */
				entry_load->seen = (int)args[1][0] - (int)'0';
				entry_load->downloaded = (int)args[2][0] - (int)'0';
			}
			entry_load = entry_load->next;
		}
	}
	
	fclose(fp);

	return 0;
}

int LOAD_ModifyGroups(int groupid, const char * groupname, short action){
	FILE * fp;
	char buffer[256];
	
	short doesFileExist = 1;
	short isInsideGroupSection = 0;
	short isGroupnameArgumentMatched = 0;
	int matchedLineNum = 0;
	int getGroupId = -1;
			
	if((fp = fopen(pth_gConf, "rb")) != NULL){
		int lineCount = 0;
		while(fgets(buffer, sizeof(buffer), fp)){
			lineCount++;

			int i=0;
			short newarg = 0;
			short arguments_count = 0;
			char * arguments[3] = {0};
	
			if(buffer[0] == '#')
				continue;
			
			size_t len = strlen(buffer);
			while(len > 0 && (buffer[len-1] == '\n' || buffer[len-1] == '\r')){
				buffer[len-1] = 0;
				len--;
			}

			if(!isInsideGroupSection){
				isInsideGroupSection=(strcmp(buffer, "@groups")==0);
				continue;
			}
			
			arguments[0] = &buffer[0];
			short was_space = 0;
			while(buffer[i] != 0){
				if(buffer[i] == ' '){
					buffer[i] = 0;
					was_space = 1;
				}
				else if(was_space){
					was_space = 0;
					arguments[++arguments_count] = &buffer[i];
					if(arguments_count == 2) break;
				}
				i++;
			}
			
			if(strcmp(arguments[0], "register-group") == 0){
				if(arguments_count==2){  
					int id = atoi(arguments[1]);
					if(id == groupid){
						matchedLineNum = lineCount;
						getGroupId = id;
					}

					if(groupname!=NULL && strcmp(arguments[2], groupname)==0){
						isGroupnameArgumentMatched = 1;
					}
				}
			}
		}
		fclose(fp);
	} else doesFileExist = 0;
	
	if(action == 1){ // Register-group _ groupid arg is the assigned id
		if(isGroupnameArgumentMatched==0 && groupname!=NULL){
			FILE * fp = fopen(pth_gConf,"ab");
			if(fp){
				if(!doesFileExist) fprintf(fp,"@groups");
				fprintf(fp, "\nregister-group %d %s\n", groupid, groupname);
			}
			fclose(fp);

			cat_group *new_group = malloc(sizeof(cat_group));
			new_group->id = groupid;
			strncpy(new_group->name, groupname, 20);
			new_group->name[20] = '\0';
			new_group->next = NULL;
			new_group->first_member = NULL;
			new_group->count = 0;

			if (initial_group == NULL){
				initial_group = new_group;
			} else {
				cat_group *tail = initial_group;
				while (tail->next) tail = tail->next;
				tail->next = new_group;
			}
			return 0; // Success
		}
	}
	
	if ((action == 2 || action == 3) && doesFileExist == 1 && matchedLineNum > 0){
		if (action == 3 && isGroupnameArgumentMatched) return 1; // Cannot rename to a name that already exists

		char tmp_path[256];
		snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", pth_gConf);

		FILE *fin = fopen(pth_gConf, "rb");
		FILE *fout = fopen(tmp_path, "wb");
		if (fin && fout){
			char line[256];
			int lineIdx = 0;
			while(fgets(line, sizeof(line), fin)){
				lineIdx++;
				if (lineIdx == matchedLineNum){
					if (action == 3){ // Rename
						fprintf(fout, "register-group %d %s\n", groupid, groupname);
					}
					// If action == 2 (Remove), we just skip writing the line entirely!
				} else {
					fputs(line, fout);
				}
			}
			fclose(fin);
			fclose(fout);
			
			// Replace old config with new config
			remove(pth_gConf);
			rename(tmp_path, pth_gConf);

			// Update in-memory linked list
			cat_group *prev = NULL;
			cat_group *curr = initial_group;
			while(curr){
				if(curr->id == groupid){
					if(action == 2){
						// Group removed globally
						LOAD_ModifyWebfeeds(NULL, curr->name, NULL, 2);
						if(prev) prev->next = curr->next;
						else initial_group = curr->next;
						free(curr); // Free the memory!
					} else if (action == 3){
						// Group renamed globally
						LOAD_ModifyWebfeeds(NULL, curr->name, groupname, 1);
						strncpy(curr->name, groupname, 20);
						curr->name[20] = '\0';
					}
					break;
				}
				prev = curr;
				curr = curr->next;
			}
			return 0; // Success
		}
		if (fin) fclose(fin);
		if (fout) fclose(fout);
	}

	return 1;	
}

int LOAD_ModifyWebfeeds(const char * target_url, const char * old_groupname, const char * new_groupname, short action){
	int target_found = 0;
	int group_removed = 0;
	
	if (action == 5){ // Add Feed
		FILE *fp = fopen(pth_Webfeeds, "ab");
		if (fp){
			fprintf(fp, "\n%s", target_url);
			fclose(fp);
			LOAD_GetWebfeedsList(); // Update autocomplete
			return 0;
		}
		return 1;
	}

	char tmp_path[256];
	snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", pth_Webfeeds);

	FILE *fin = fopen(pth_Webfeeds, "rb");
	if(!fin) return 1;

	FILE *fout = fopen(tmp_path, "wb");
	if(!fout){
		fclose(fin);
		return 1;
	}

	char line[512];
	while(fgets(line, sizeof(line), fin)){
		size_t len = strlen(line);
		while(len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')){
			line[len-1] = 0;
			len--;
		}
		if (len == 0) continue;
		
		char url[256] = {0};
		char group[256] = {0};
		char alias[128] = {0};
		int has_group = 0;
		int has_alias = 0;

		char *first_space = strchr(line, ' ');
		if (first_space){
			int u_len = first_space - line;
			strncpy(url, line, u_len);
			url[u_len] = 0;
			
			char *metadata = first_space;
			
			char *bracket_start = strchr(metadata, '[');
			if (bracket_start){
				char *bracket_end = strchr(bracket_start, ']');
				if (bracket_end){
					has_group = 1;
					int g_len = bracket_end - bracket_start - 1;
					strncpy(group, bracket_start + 1, g_len);
					group[g_len] = 0;
				}
			}
			
			char *paren_start = strchr(metadata, '(');
			if (paren_start){
				char *paren_end = strchr(paren_start, ')');
				if (paren_end){
					has_alias = 1;
					int a_len = paren_end - paren_start - 1;
					strncpy(alias, paren_start + 1, a_len);
					alias[a_len] = 0;
				}
			}
		} else {
			int u_len = strlen(line);
			while(u_len > 0 && line[u_len-1] == ' ') u_len--;
			strncpy(url, line, u_len);
			url[u_len] = 0;
		}

		char *url_stripped = url;
		if (strncmp(url_stripped, "https://", 8) == 0) url_stripped += 8;
		else if (strncmp(url_stripped, "http://", 7) == 0) url_stripped += 7;
		if (strncmp(url_stripped, "www.", 4) == 0) url_stripped += 4;

		const char *target_stripped = target_url;
		if (target_stripped){
			if (strncmp(target_stripped, "https://", 8) == 0) target_stripped += 8;
			else if (strncmp(target_stripped, "http://", 7) == 0) target_stripped += 7;
			if (strncmp(target_stripped, "www.", 4) == 0) target_stripped += 4;
		}

		int is_target_url = (target_stripped && (strcmp(url_stripped, target_stripped) == 0 || (has_alias && _stricmp(alias, target_url) == 0)));
		
		if (action == 7){ // Set Alias
			if (!is_target_url){
				fprintf(fout, "%s", url);
				if (has_alias) fprintf(fout, " (%s)", alias);
				if (has_group) fprintf(fout, " [%s]", group);
				fprintf(fout, "\n");
				continue;
			}
			target_found = 1;
			fprintf(fout, "%s", url);
			if (new_groupname && strlen(new_groupname) > 0) fprintf(fout, " (%s)", new_groupname);
			if (has_group) fprintf(fout, " [%s]", group);
			fprintf(fout, "\n");
			continue;
		} else if (action == 6){ // Remove Feed
			if (is_target_url){
				target_found = 1;
				continue; // Skip writing entirely!
			}
		} else if (action == 3 || action == 4){
			if (!is_target_url){
				fprintf(fout, "%s", url);
				if (has_alias) fprintf(fout, " (%s)", alias);
				if (has_group) fprintf(fout, " [%s]", group);
				fprintf(fout, "\n");
				continue;
			}
			target_found = 1;
		}

		if (action >= 1 && action <= 4){
			// If action 3, we proceed even if has_group is 0 so we can add it
			if (has_group || action == 3){
				char group_copy[256] = {0};
				if (has_group) strcpy(group_copy, group);

				char new_group_str[256] = {0};
				int first = 1;
				int found = 0;
				
				if (has_group){
					char *token = strtok(group_copy, ",");
					while (token != NULL){
						while(*token == ' ') token++;
						int tlen = strlen(token);
						while(tlen > 0 && token[tlen-1] == ' '){ token[tlen-1] = 0; tlen--; }

						if (action == 1 || action == 2){
							if (_stricmp(token, old_groupname) == 0){
								found = 1;
								if (action == 1){ // Global Rename
									if (!first) strcat(new_group_str, ",");
									strcat(new_group_str, new_groupname);
									first = 0;
								}
							} else {
								if (!first) strcat(new_group_str, ",");
								strcat(new_group_str, token);
								first = 0;
							}
						} else if (action == 3 || action == 4){
							// Use new_groupname as the target group to add/remove for the URL
							if (_stricmp(token, new_groupname) == 0){
								found = 1;
								if (action == 3){ // Add (already exists, so we just keep it)
									if (!first) strcat(new_group_str, ",");
									strcat(new_group_str, token);
									first = 0;
								}
								// If action 4 (Remove), we skip adding it to new_group_str!
							} else {
								if (!first) strcat(new_group_str, ",");
								strcat(new_group_str, token);
								first = 0;
							}
						}
						token = strtok(NULL, ",");
					}
				}

				if (action == 3 && !found){ // Append the new group!
					if (!first) strcat(new_group_str, ",");
					strcat(new_group_str, new_groupname);
					found = 1;
				}

				if (action == 4 && found){
					group_removed = 1; // Mark that we successfully removed it
				}

				if ((action == 1 || action == 2) && !found){
					// Global action but old group not found on this URL, write original
					fprintf(fout, "%s", url);
					if (has_alias) fprintf(fout, " (%s)", alias);
					if (has_group) fprintf(fout, " [%s]", group);
					fprintf(fout, "\n");
					continue;
				}

				if (strlen(new_group_str) > 0){
					fprintf(fout, "%s", url);
					if (has_alias) fprintf(fout, " (%s)", alias);
					fprintf(fout, " [%s]\n", new_group_str);
				} else {
					fprintf(fout, "%s", url);
					if (has_alias) fprintf(fout, " (%s)", alias);
					fprintf(fout, "\n");
				}
				continue;
			}
		}
		
		// If no modifications were made, just print the original extracted components
		if (has_group){
			fprintf(fout, "%s", url);
			if (has_alias) fprintf(fout, " (%s)", alias);
			fprintf(fout, " [%s]\n", group);
		} else {
			fprintf(fout, "%s", url);
			if (has_alias) fprintf(fout, " (%s)", alias);
			fprintf(fout, "\n");
		}
	}
	fclose(fin);
	fclose(fout);
	
	if ((action == 3 || action == 4 || action == 6 || action == 7) && !target_found){
		remove(tmp_path);
		return 2; // Error: URL not found
	}
	if (action == 4 && !group_removed){
		remove(tmp_path);
		return 3; // Error: Group not found in URL
	}

	remove(pth_Webfeeds);
	rename(tmp_path, pth_Webfeeds);
	
	LOAD_GetWebfeedsList(); // Refresh autocomplete cache!
	return 0;
}

