/** Initialize evilpoison commands.
 * This function sets up all default internal commands.
 */
void evilpoison_commands_init( void );

char *evilpoison_command_set(char *commandline);
char *evilpoison_command_unset(char *commandline);

char *evilpoison_command_bind(char *commandline);
char *evilpoison_command_cmdmode(char *commandline);
char *evilpoison_command_exec(char *commandline);
char *evilpoison_command_info(char *commandline);

char *evilpoison_command_nextwin(char *commandline);

char *evilpoison_command_max(char *commandline);
char *evilpoison_command_maxhoriz(char *commandline);
char *evilpoison_command_maxvert(char *commandline);
