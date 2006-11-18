/** Settings header.
 * \file settings.h
 */

/** Initialize settings.
 * This function should be called once only at or near the beginning of the
 * application's startup. It will initialize all internal storage and prepare
 * itself for runtime operation. It is not absolutely neccessary to run this
 * right at the start, as long as this function is called before any others.
 */
void settings_init( void );

/** Set a key-value pair
 * This function will either add a new setting, or overwrite an existing
 * setting depending on the setting's prior existance.
 * \param key The key with which to associate the given value.
 * \param value The value to be stored and associated with the given key.
 */
void settings_set( const char *key, const char *value );

/** Check whether a key exists.
 * The given key will be checked for existance.
 * \param key This is the key whose existance is under question.
 * \return A boolean, zero or non-zero, result indicating the given key's
 * existance.
 */
unsigned int settings_haskey( const char *key );

/** Retrieve the value associated with the given key.
 * \param key This is the key for which the associated data will be retrieved.
 * \return The value (char *) which was associated with this key, or null
 * (zero) if this key does not exist.
 */
char *settings_get( const char *key );

/** Unset, or clear, a key-value pair.
 * The data which was previously stored at this key will be deleted.
 * \param key The key for which the associated data will be deleted.
 */
void settings_unset( const char *key );
