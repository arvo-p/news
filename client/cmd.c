int cmd_mode = 0;
int cmd_count = 0;
char cmd_buffer[100];

char * cHelper;
int cHelper_len=0;
int cHelper_max=0;
char cError[115];

int run_cmd(int helper){
	if(strlen(cmd_buffer) < 3) return 0;

	int err = 1;
	int helper_found = 0;

	char * process_cmd = malloc(cmd_count+1);
	char * args[10];
	int args_c=0;
	int args_num=0;

	strcpy(process_cmd, cmd_buffer+1);
	
	args[args_c++] = process_cmd;

	int empty = 0;
	for(int i=0;i<cmd_count;i++){
		if(process_cmd[i] == ' '){
			process_cmd[i] = 0;
			empty = 1;
		}
		else if(empty){
			empty = 0;
			args[args_c++] = process_cmd+i;
		}
	}

	for(int i=0;i<args_c;i++) if(args[i][0]) args_num++;
	
	if(strcmp(args[0], "open-group") == 0 || args[0][0] == 'g'){
		/*
		 * This command quits the program if the requested group is empty.
		 */
		if(args_num == 2){
			err = 0;
			int len = strlen(args[1]);
			cat_group * findGroup = initial_group;
			while(findGroup){
				if(!helper && strcasecmp(findGroup->name, args[1]) == 0) tabs_openGroup(findGroup);
			
				if(helper && len>0){
					if(strncasecmp(findGroup->name, args[1], len) == 0){
						cHelper = findGroup->name;
						cHelper_len = len-1;
						helper_found = 1;
					}
				}
			
				findGroup = findGroup->next;
			}
		} else err = 2;
	}

	if(helper){
		if(!helper_found){
			cHelper = NULL;
		}
		err = 0;
	}else{
		cHelper = NULL;
	}

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

int cmd_autocomplete(){
	if(!cHelper) return 1;
	strcat(cmd_buffer, cHelper+cHelper_len+1);
	cmd_count += strlen(cHelper)-cHelper_len-1;
	cHelper = NULL;
	return 0;
}

void exit_command_mode(){
	printf("\e[%d;0H", winSZ[1]);
	printf("\e[2K");
	cmd_mode = 0;
}
