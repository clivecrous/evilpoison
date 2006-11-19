/** EvilPoison Commands header.
 * \file evilpoison_commands.h
 */

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

/** window.move.mouse : Move a window with the mouse.
 * \note This command will eventually be deprecated in favour of
 *    window.move.mouse.start and window.move.mouse.stop once bindings allow for
 *    binding to mouse events.
 * \param commandline currently unused.
 * \return This should always return NULL.
 */
char *evilpoison_command_window_move_mouse(char *commandline);

/** window.resize.mouse : Move a window with the mouse.
 * \note This command will eventually be deprecated in favour of
 *    window.resize.mouse.start and window.resize.mouse.stop once bindings allow
 *    for binding to mouse events.
 * \param commandline currently unused.
 * \return This should always return NULL.
 */
char *evilpoison_command_window_resize_mouse(char *commandline);

#ifdef VWM
char *evilpoison_command_fix(char *commandline);
char *evilpoison_command_prevdesk(char *commandline);
char *evilpoison_command_nextdesk(char *commandline);
#endif

/** window.move : Move a window by the given amount.
 * The commandline consists of an X and Y co-ordinate as the followng examples
 * demonstrate.
 *    Move the window right 10 pixels and down 20 pixels:
 *      window.move 10 20
 *    Move the window left 5 pixels:
 *      window.move -5 0
 * \param commandline consists of an X and Y delta co-ordinate by which the
 *    window's location is to be adjusted.
 * \return This should always return NULL.
 */
char *evilpoison_command_window_move(char *commandline);

/** window.moveto : Move a window to an exact location.
 * The commandline consists of an X and Y co-ordinate as the followng examples
 * demonstrate.
 *    Move the window to the location 10,20 :
 *      window.moveto 10 20
 *    Move the window to the location 20 pixels from the right of the screen
 *    and 10 pixels above the bottom of the screen:
 *      window.moveto -20 -10
 *    Move the window 50 pixels from the left of the screen and centered
 *    vertically:
 *      window.moveto 50 X
 * \param commandline consists of an X and Y co-ordinate to which the
 *    window's location is to be moved.
 * \return This should always return NULL.
 */
char *evilpoison_command_window_moveto(char *commandline);

/** window.resize : Resize a window by the given amount.
 * window.resize <top> <left> <right> <bottom>
 * The commandline consists of an Top,Left,Right and Bottom co-ordinate deltas
 * as the followng examples demonstrate.
 *    Resize the window right hand side border 10 pixels to the right:
 *      window.resize 0 0 10 0
 *    Resize the window left border 5 pixels to the left:
 *      window.resize -5 0 0 0
 * \param commandline consists of a Top,Left,Right and Bottom delta by which the
 *    window's size will be adjusted.
 *    window's location is to be adjusted.
 * \return This should always return NULL.
 */
char *evilpoison_command_window_resize(char *commandline);
