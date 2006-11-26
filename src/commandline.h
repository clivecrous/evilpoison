/** CommandLine header.
 * \file commandline.h
 */

/** Initialize commandline.
 * This function should be called once only at or near the beginning of the
 * application's startup. It will initialize all commandline storage and prepare
 * itself for runtime operation. It is not absolutely neccessary to run this
 * right at the start, as long as this function is called before any others.
 */
void commandline_init( void );

/** Add a commandline setting.
 * \param key The settings key with which to associate the given commandline
 *    option.
 * \param option_short The short form of this commandline option.   
 * \param option_long The long form of this commandline option.   
 * \param description_short The short description of this option, as displayed
 *    when --help is asked for.
 * \param description_long The long description of this option, as displayed
 *    when --help <option> is asked for. This can be NULL (if none).
 * \param value_default The value to be applied when this option is not
 *    selected.
 * \param value_set The value to be applied when this option is selected. If
 *    this is NULL, then the user is expected to pass a value on the
 *    commandline.
 */
void commandline_add(
    const char *key,
    const char *option_short,
    const char *option_long,
    const char *description_short,
    const char *description_long,
    const char *value_default,
    const char *value_set );

/** Process the commandline.
 * This function processes the given commandline in accordance to the added
 * commandline information.
 * \param argc The amount of arguments.
 * \param arcv The arguments as given to main.
 * \return A boolean indicating success or failure. Calling program would
 *    generally abort on failure. A failure will automatically print out
 *    commandline help to stdout before returning.
 */
unsigned int commandline_process( unsigned int argc, char *argv[] );
