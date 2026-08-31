#ifndef CMD_H
#define CMD_H

typedef int (*CmdFunc)(int args_num, char **args, int helper);

typedef struct{
	const char * name;
	const char * alias;
	CmdFunc func;
} CommandDef;

int run_cmd(int helper);
int cmd_autocomplete();
void exit_command_mode();

extern int cmd_mode;
extern int cmd_count;
extern char cmd_buffer[100];

extern char * cHelper;
extern int cHelper_selected_cmd;
extern int cHelper_len;
extern int cHelper_max;
extern char cError[115];

#endif
