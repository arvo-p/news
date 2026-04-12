#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <shlobj.h>
#include <direct.h>
#include "headers/loadfiles.h"
#include "headers/main.h"
#include "headers/ui.h"

char *pth_gConf, *pth_folder_colorscheme, *pth_Root, * pth_EntriesInfo, * pth_EntriesInfoMod, * pth_mainNews, * pth_Archive;
short homepageExcludedGroups[32];

char * pathAppend(char * pth, char * toAppend){
	int len = strlen(pth);
	int addDir = 0;
	if(pth[len-1] != '\\') addDir=1;

	char * newstr = malloc(len+strlen(toAppend)+addDir+1);
	
	snprintf(newstr, len + strlen(toAppend) + 2, "%s%s%s", pth, addDir ? "\\" : "", toAppend);
	return newstr;
}

void get_Filepaths(){
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
	pth_Archive = pathAppend(pth_Root, folder_archive);
	pth_folder_colorscheme = pathAppend(pth_Root, folder_colorscheme);

	return;
}

int LoadCategoryGroups(){
	FILE * fp;	
	char line[320];

	cat_group * first = NULL;
	cat_group * previous = NULL;

	fp = fopen(pth_gConf, "rb");
	if(!fp) return 1;
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

int LoadEntry(char * line, entry * new_entry, entry ** previous_entry, int * n_parent){
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

int LoadEntries(){
	/*
	 * Entries are loaded upside down in memory.
	 * Because the server appends new entries at the end of the file.
	 */

	entry * previous_entry = NULL;
	int n_parent = 0;
	chunk * entry_block = NULL;
	
	FILE * fp;
	char line[400];

	entries_sz = 0;

	fp = fopen(pth_mainNews, "rb");
	if(!fp) return 1;

	int i=0;
	while (fgets(line, sizeof(line), fp)){				
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

		LoadEntry(line, &entry_block->entry[95-(i%96)], &previous_entry, &n_parent);
		i++;
	}
	if(entry_block) entry_block->start_index = 95-((i-1)%96);
	initial_entry = previous_entry;	

	fclose(fp);
	return 0;
}

int updateInfoFromFile(entry * target){
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

int loadInfoFromFile(){
	FILE * fp;
	char line[320];

	fp = fopen(pth_EntriesInfo, "rb");
	if(!fp) return 1;

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
