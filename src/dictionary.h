typedef struct {
  char *key;
  void *value;
} DictionaryPair;

typedef struct {
  unsigned int size;
  DictionaryPair *data;
} Dictionary;

/** Create a dictionary.
 * This function creates, and returns, a Dictionary.
 */
Dictionary *dictionary_create( void );

/** Destroy a dictionary.
 * This function releases all memory associated within this dictionary, for
 * keys as well as values and also for the dictionary itself.
 * \param dictionary The dictionary you wish to destroy.
 */
void dictionary_destroy( Dictionary *dictionary );

/** Set a key-value pair
 * This function will either add a new key-value pair, or overwrite an existing
 * depending on the it's prior existance.
 * \param dictionary The dictionary you wish to alter.
 * \param key The key with which to associate the given value.
 * \param value The value to be stored and associated with the given key.
 */
void dictionary_set( Dictionary *dictionary, const char *key, void *value );

/** Check whether a key exists.
 * The given key will be checked for existance.
 * \param dictionary The dictionary you wish to check.
 * \param key This is the key whose existance is under question.
 * \return A boolean, zero or non-zero, result indicating the given key's
 * existance.
 */
unsigned int dictionary_haskey( Dictionary *dictionary, const char *key );

/** Retrieve the value associated with the given key.
 * \param dictionary The dictionary you wish to retrieve from.
 * \param key This is the key for which the associated data will be retrieved.
 * \return The value (void *) which was associated with this key, or null
 * (zero) if this key does not exist.
 */
void *dictionary_get( Dictionary *dictionary, const char *key );

/** Unset, or clear, a key-value pair.
 * The data which was previously stored at this key will be deleted.
 * \param dictionary The dictionary you wish to remove an item from.
 * \param key The key for which the associated data will be deleted.
 */
void dictionary_unset( Dictionary *dictionary, const char *key );
