int run_cmd(int helper);
int cmd_autocomplete();
void exit_command_mode();

extern int cmd_mode;
extern int cmd_count;
extern char cmd_buffer[100];

extern char * cHelper;
extern int cHelper_len;
extern int cHelper_max;
extern char cError[115];


