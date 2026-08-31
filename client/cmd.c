#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "headers/cmd.h"
#include "headers/main.h"
#include "headers/tabs.h"
#include "headers/ui.h"

int cmd_mode = 0;
int cmd_count = 0;
char cmd_buffer[100];

char * cHelper;
int cHelper_len=0;
int cHelper_max=0;
int cHelper_selected_cmd = 0;
char cError[115];
int helper_found = 0;

int cmd_open_group(int args_num, char **args, int helper);
int cmd_register_group(int args_num, char **args, int helper);
int cmd_rename_group(int args_num, char **args, int helper);
int cmd_remove_group(int args_num, char **args, int helper);
int cmd_feed_group(int args_num, char **args, int helper);
int cmd_feed_ungroup(int args_num, char **args, int helper);
int cmd_feed_add(int args_num, char **args, int helper);
int cmd_feed_remove(int args_num, char **args, int helper);
int cmd_feed_alias(int args_num, char **args, int helper);
int cmd_feeds(int args_num, char **args, int helper);

CommandDef commands[] = {
	{"group-open", "g", cmd_open_group},
	{"group-register", "greg", cmd_register_group},
	{"group-rename", "gren", cmd_rename_group},
	{"group-remove", "grem", cmd_remove_group},
	{"feed-group", "fg", cmd_feed_group},
	{"feed-ungroup", "fu", cmd_feed_ungroup},
	{"feed-add", "fa", cmd_feed_add},
	{"feed-remove", "fr", cmd_feed_remove},
	{"feed-alias", "fal", cmd_feed_alias},
	{"feeds", "f", cmd_feeds},
};

void cmd_provide_suggestions(const char **options, int num_options, const char *typed){
    int typed_len = strlen(typed);

    int count = 0; // Count matches
    for(int i = 0; i < num_options; i++)
        if(strncasecmp(options[i], typed, typed_len) == 0) count++;

    if(count > 0){ // Boundaries
        if(cHelper_selected_cmd >= count) cHelper_selected_cmd = 0;
        if(cHelper_selected_cmd < 0) cHelper_selected_cmd = count - 1;
    }else{
        cHelper_selected_cmd = 0;
        return;
    }

    int current = 0; // Select suggestion
    for(int i = 0; i < num_options; i++){
        if(strncasecmp(options[i], typed, typed_len) == 0){
            if(current == cHelper_selected_cmd){
                cHelper = (char *)options[i];
                cHelper_len = typed_len - 1;
                helper_found = 1;
                return;
            }
            current++;
        }
    }
}

int run_cmd(int helper){
	if(strlen(cmd_buffer) < 3) return 0;

	int err = 1;
	helper_found = 0;

	char * process_cmd = malloc(cmd_count+1);
	char * args[10];
	int args_c=0;
	int args_num=0;

	strcpy(process_cmd, cmd_buffer+1);
	
	args[args_c++] = process_cmd;

	int empty = 0;
	int in_quotes = 0;
	for(int i=0;i<cmd_count;i++){
		if (process_cmd[i] == '"'){
			if (empty){
				empty = 0;
				args[args_c++] = process_cmd+i;
			}
			in_quotes = !in_quotes;
		} else if(process_cmd[i] == ' ' && !in_quotes){
			process_cmd[i] = 0;
			empty = 1;
		}
		else if(empty){
			empty = 0;
			args[args_c++] = process_cmd+i;
		}
	}

	for(int i=0;i<args_c;i++){
		if(args[i][0]) args_num++;
		if(args[i][0] == '"'){
			args[i]++;
			int l = strlen(args[i]);
			if(l > 0 && args[i][l-1] == '"') args[i][l-1] = 0;
		}
	}
	
	int has_trailing_space = (cmd_count > 0 && cmd_buffer[cmd_count-1] == ' ' && !in_quotes);
	if (helper && has_trailing_space){
		args[args_c] = "";
		args_c++;
		args_num++;
	}
	
	int len = strlen(args[0]);
	int cmd_array_size = (sizeof(commands) / sizeof(CommandDef));
	
	if(len <= 0){
		free(process_cmd);
		return 0;
	}
	
	if(helper){
		int exact_alias_match = 0;
		
		for(int i=0; i<cmd_array_size; i++){
			if(commands[i].alias && strcmp(args[0], commands[i].alias) == 0){
				exact_alias_match = 1;
				break;
			}
		}

		if(args_num == 1 && !has_trailing_space && !exact_alias_match){
			const char *cmd_names[50];
			int num_cmds = 0;
			for(int i = 0; i < cmd_array_size; i++){
				cmd_names[num_cmds++] = commands[i].name;
			}
			cmd_provide_suggestions(cmd_names, num_cmds, args[0]);
		}
	}
	
	for(int i=0;i<cmd_array_size;i++){
			int call_command = 0;
		
			if(commands[i].alias) call_command = (strcmp(args[0], commands[i].alias) == 0);
			call_command = (call_command==0)?(strcmp(args[0], commands[i].name) == 0):1;
			
			if(call_command){
				err = commands[i].func(args_num, args, helper); 
				break;
			}
	}

	if(helper){
		if(!helper_found)
			cHelper = NULL;
		err = 0;
	}else cHelper = NULL;

	switch(err){
		case 1:
			strcpy(cError, "Not a command: ");
			strcat(cError, process_cmd);	
			break;
		case 2:
			sprintf(cError, "Incorrect number of arguments: %d", args_num);
			break;
	}

	free(process_cmd);
	return err;
}

int cmd_open_group(int args_num, char **args, int helper){
	if(args_num == 2){
		if(helper){
			const char *group_names[100];
			int num_groups = 0;
			cat_group * findGroup = initial_group;
			while(findGroup && num_groups < 100){
				group_names[num_groups++] = findGroup->name;
				findGroup = findGroup->next;
			}
			cmd_provide_suggestions(group_names, num_groups, args[1]);
		}else{
			cat_group * findGroup = initial_group;
			while(findGroup){
				if(strcasecmp(findGroup->name, args[1]) == 0){
					tabs_openGroup(findGroup);
					break;
				}
				findGroup = findGroup->next;
			}
		}
		return 0;
	} else return 2;
}

int cmd_register_group(int args_num, char **args, int helper){
	// Usage: :group-register <name>
	if(args_num == 2){
		if(!helper){

			int max_id = 0;
			cat_group *findGroup = initial_group;
			while(findGroup){
				if(findGroup->id > max_id) max_id = findGroup->id;
				findGroup = findGroup->next;
			}
			int new_id = max_id + 1;

			// Write it to the config file!
			int res = Load->ModifyGroups(new_id, args[1], 1);

			if (res == 0){
				sprintf(statusbar_notify, "Group '%s' created with ID %d!", args[1], new_id);
			} else {
				sprintf(statusbar_notify, "Group '%s' already exists!", args[1]);
			}
			
			draw_statusbar();
			ui_flush();
		}
		return 0;
	} else return 2;
}

int cmd_rename_group(int args_num, char **args, int helper){
	if(helper){
		if(args_num == 2){
			const char *group_names[100];
			int num_groups = 0;
			cat_group * findGroup = initial_group;
			while(findGroup && num_groups < 100){
				group_names[num_groups++] = findGroup->name;
				findGroup = findGroup->next;
			}
			cmd_provide_suggestions(group_names, num_groups, args[1]);
		}
		return 0;
	}

	if(args_num == 3){
		int target_id = -1;
		cat_group *findGroup = initial_group;
		while(findGroup){
			if(strcasecmp(findGroup->name, args[1]) == 0){
				target_id = findGroup->id;
				break;
			}
			findGroup = findGroup->next;
		}

		if (target_id != -1){
			int res = Load->ModifyGroups(target_id, args[2], 3); // 3 for rename
			if(res == 0){
				sprintf(statusbar_notify, "Group renamed to '%s'!", args[2]);
			}else{
				sprintf(statusbar_notify, "Error renaming group '%s'!", args[1]);
			}
		}else{
			sprintf(statusbar_notify, "Group '%s' not found!", args[1]);
		}
		draw_statusbar();
		ui_flush();
		return 0;
	}
	return 2;
}

int cmd_remove_group(int args_num, char **args, int helper){
	if(args_num == 2){
		if(helper){
			const char *group_names[100];
			int num_groups = 0;
			cat_group * findGroup = initial_group;
			while(findGroup && num_groups < 100){
				group_names[num_groups++] = findGroup->name;
				findGroup = findGroup->next;
			}
			cmd_provide_suggestions(group_names, num_groups, args[1]);
		}else{
			int target_id = -1;
			cat_group *findGroup = initial_group;
			while(findGroup){
				if(strcasecmp(findGroup->name, args[1]) == 0){
					target_id = findGroup->id;
					break;
				}
				findGroup = findGroup->next;
			}

			if(target_id != -1){
				int res = Load->ModifyGroups(target_id, args[1], 2); // 2 for remove
				if(res == 0){
					sprintf(statusbar_notify, "Group '%s' removed!", args[1]);
				}else{
					sprintf(statusbar_notify, "Error removing group '%s'!", args[1]);
				}
			}else{
				sprintf(statusbar_notify, "Group '%s' not found!", args[1]);
			}
			draw_statusbar();
			ui_flush();
		}
		return 0;
	}
	return 2;
}

int cmd_feed_group(int args_num, char **args, int helper){
	if (helper){
		if (args_num == 2){
			const char *options[400];
			int num = 0;
			for (int i = 0; i < webfeeds_count; i++){
				if (strlen(webfeeds_list[i].alias) > 0) options[num++] = webfeeds_list[i].alias;
				options[num++] = webfeeds_list[i].url;
			}
			if (selected_tab->tab_mode == TAB_WEBFEEDS){
				cat_group * curr = initial_group;
				while(curr && num < 400){
					options[num++] = curr->name;
					curr = curr->next;
				}
			}
			cmd_provide_suggestions(options, num, args[1]);
		} else if (args_num == 3){
			const char *groups[100];
			int num = 0;
			cat_group * curr = initial_group;
			while(curr && num < 100){
				groups[num++] = curr->name;
				curr = curr->next;
			}
			cmd_provide_suggestions(groups, num, args[2]);
		}
		return 0;
	}

	char *target_url = NULL;
	char *target_group = NULL;

	if (args_num == 3){
		target_url = args[1];
		target_group = args[2];
	} else if (args_num == 2 && selected_tab->tab_mode == TAB_WEBFEEDS){
		int idx = selected_tab->line_offset + selected_tab->sel;
		if (idx < webfeeds_count){
			target_url = webfeeds_list[idx].url;
			target_group = args[1];
		}
	}

	if (target_url && target_group){
		int res = Load->ModifyWebfeeds(target_url, NULL, target_group, 3); // 3 for Add
		if (res == 0){
			sprintf(statusbar_notify, "Group '%s' added to feed!", target_group);
		} else if (res == 2){
			sprintf(statusbar_notify, "Error: URL '%s' not found!", target_url);
		} else {
			sprintf(statusbar_notify, "Error adding group to feed!");
		}
		draw_update(true);
		return 0;
	}
	return 2;
}

int cmd_feed_ungroup(int args_num, char **args, int helper){
	if (helper){
		if (args_num == 2){
			const char *options[400];
			int num = 0;
			for (int i = 0; i < webfeeds_count; i++){
				if (strlen(webfeeds_list[i].alias) > 0) options[num++] = webfeeds_list[i].alias;
				options[num++] = webfeeds_list[i].url;
			}
			if (selected_tab->tab_mode == TAB_WEBFEEDS){
				cat_group * curr = initial_group;
				while(curr && num < 400){
					options[num++] = curr->name;
					curr = curr->next;
				}
			}
			cmd_provide_suggestions(options, num, args[1]);
		} else if (args_num == 3){
			const char *groups[100];
			int num = 0;
			cat_group * curr = initial_group;
			while(curr && num < 100){
				groups[num++] = curr->name;
				curr = curr->next;
			}
			cmd_provide_suggestions(groups, num, args[2]);
		}
		return 0;
	}

	char *target_url = NULL;
	char *target_group = NULL;

	if (args_num == 3){
		target_url = args[1];
		target_group = args[2];
	} else if (args_num == 2 && selected_tab->tab_mode == TAB_WEBFEEDS){
		int idx = selected_tab->line_offset + selected_tab->sel;
		if (idx < webfeeds_count){
			target_url = webfeeds_list[idx].url;
			target_group = args[1];
		}
	}

	if (target_url && target_group){
		int res = Load->ModifyWebfeeds(target_url, NULL, target_group, 4); // 4 for Remove
		if (res == 0){
			sprintf(statusbar_notify, "Group '%s' removed from feed!", target_group);
		} else if (res == 2){
			sprintf(statusbar_notify, "Error: URL '%s' not found!", target_url);
		} else if (res == 3){
			sprintf(statusbar_notify, "Feed is not in group '%s'!", target_group);
		} else {
			sprintf(statusbar_notify, "Error removing group from feed!");
		}
		draw_update(true);
		return 0;
	}
	return 2;
}

int cmd_feed_add(int args_num, char **args, int helper){
	if (helper) return 0; // No autocomplete for a brand new URL
	
	if (args_num == 2){
		int res = Load->ModifyWebfeeds(args[1], NULL, NULL, 5); // 5 for Add
		if (res == 0){
			sprintf(statusbar_notify, "Feed '%s' added!", args[1]);
		} else {
			sprintf(statusbar_notify, "Error adding feed!");
		}
		draw_update(true);
		return 0;
	}
	return 2;
}

int cmd_feed_remove(int args_num, char **args, int helper){
	if (helper){
		if (args_num == 2){
			const char *urls[400];
			int num = 0;
			for (int i = 0; i < webfeeds_count; i++){
				if (strlen(webfeeds_list[i].alias) > 0) urls[num++] = webfeeds_list[i].alias;
				urls[num++] = webfeeds_list[i].url;
			}
			cmd_provide_suggestions(urls, num, args[1]);
		}
		return 0;
	}

	char *target_url = NULL;

	if (args_num == 2){
		target_url = args[1];
	} else if (args_num == 1 && selected_tab->tab_mode == TAB_WEBFEEDS){
		int idx = selected_tab->line_offset + selected_tab->sel;
		if (idx < webfeeds_count){
			target_url = webfeeds_list[idx].url;
		}
	}

	if (target_url){
		int res = Load->ModifyWebfeeds(target_url, NULL, NULL, 6); // 6 for Remove
		if (res == 0){
			sprintf(statusbar_notify, "Feed removed!");
		} else if (res == 2){
			sprintf(statusbar_notify, "Error: URL not found!");
		} else {
			sprintf(statusbar_notify, "Error removing feed!");
		}
		draw_update(true);
		return 0;
	}
	return 2;
}

int cmd_feed_alias(int args_num, char **args, int helper){
	if (helper){
		if (args_num == 2){
			const char *urls[400];
			int num = 0;
			for (int i = 0; i < webfeeds_count; i++){
				if (strlen(webfeeds_list[i].alias) > 0) urls[num++] = webfeeds_list[i].alias;
				urls[num++] = webfeeds_list[i].url;
			}
			cmd_provide_suggestions(urls, num, args[1]);
		}
		return 0;
	}

	char *target_url = NULL;
	char *target_alias = NULL;
	int is_remove = 0;

	if (args_num == 3){
		target_url = args[1];
		target_alias = args[2];
	} else if (args_num == 2){
		if (selected_tab->tab_mode == TAB_WEBFEEDS){
			int idx = selected_tab->line_offset + selected_tab->sel;
			if (idx < webfeeds_count) target_url = webfeeds_list[idx].url;
			target_alias = args[1]; // Set alias on implicit URL
		} else {
			target_url = args[1];
			is_remove = 1; // Remove alias on explicit URL
		}
	} else if (args_num == 1 && selected_tab->tab_mode == TAB_WEBFEEDS){
		int idx = selected_tab->line_offset + selected_tab->sel;
		if (idx < webfeeds_count) target_url = webfeeds_list[idx].url;
		is_remove = 1; // Remove alias on implicit URL
	}

	if (target_url){
		int res = Load->ModifyWebfeeds(target_url, NULL, is_remove ? "" : target_alias, 7);
		if (res == 0) sprintf(statusbar_notify, is_remove ? "Alias removed!" : "Alias set!");
		else sprintf(statusbar_notify, is_remove ? "Error removing alias!" : "Error setting alias!");
		draw_update(true);
		return 0;
	}
	return 2;
}

int cmd_feeds(int args_num, char **args, int helper){
	if (helper) return 0;
	selected_tab = tabs_newtab("Webfeeds", NULL, TAB_WEBFEEDS);
	draw_update(true);
	ui_flush();
	return 0;
}

int cmd_autocomplete(){
	if(!cHelper) return 1;

	int in_quotes = 0;
	for(int i = 0; i < cmd_count; i++){
		if(cmd_buffer[i] == '"') in_quotes = !in_quotes;
	}

	int has_space = strchr(cHelper, ' ') != NULL;

	strcat(cmd_buffer, cHelper+cHelper_len+1);

	if (in_quotes){
		strcat(cmd_buffer, "\" ");
		cmd_count += strlen(cHelper)-cHelper_len + 1;
	} else if (has_space){
		int start = cmd_count - 1;
		while (start >= 0 && cmd_buffer[start] != ' ') start--;
		start++;
		
		int end = strlen(cmd_buffer);
		memmove(cmd_buffer + start + 1, cmd_buffer + start, end - start + 1);
		cmd_buffer[start] = '"';
		strcat(cmd_buffer, "\" ");
		cmd_count += strlen(cHelper)-cHelper_len + 2;
	} else {
		strcat(cmd_buffer, " ");
		cmd_count += strlen(cHelper)-cHelper_len;
	}

	cHelper = NULL;
	cHelper_selected_cmd = 0;
	return 0;
}

void exit_command_mode(){
	printf("\e[%d;0H", winSZ[1]);
	printf("\e[2K");
	cmd_mode = 0;
}
