/** Command header.
 * \file command.h
 */

typedef char *(* CommandFunction)(char *);

/** Command initializer.
 * This function should be called before any other command_ functions.
 */
void command_init( void );

/** Assign a command function to a given command.
 * \param command The command to assign the given function to.
 * \param function A pointer to the function you wish to call when this command
 * is invoked.
 * \return A boolean is returned, indicating success or failure.
 */
void command_assign(const char *command, CommandFunction function);

/** Unassign a command.
 * This function unassigns the given command. This is a command-deletion.
 * \param command The command to unassign.
 */
void command_unassign(const char *command);

/** Execute a command.
 * \param commandline This is the full commandline to be executed.
 * \return A string result as returned by the command itself, or NULL.
 */
char *command_execute(const char *commandline);

/** Create a copy of a parameter.
 * The caller is resposible for freeing the memory for the string returned by
 * this function.
 * Parameter enumeration begins at zero (0).
 * \param parameters This is a whitespace seperated list of parameters.
 * \param which_parameter The parameter enumeration to create a copy of.
 * \return A copy of the parameter or NULL on failure.
 */
char *command_parameter_copy(
    const char *parameters, const unsigned int which_parameter);
