typedef struct {
  KeySym symbol;
  unsigned int mask;
} BindKeySymMask;

/** Convert a binding code string to a symbol/mask struct.
 * \param binding_code This is the string code (eg: c-t ) that you want to
 * convert into a symbol and mask combination.
 * \return A pointer to a BindKeySymMask structure containing the key binding.
 */
BindKeySymMask *keycode_convert( const char *binding_code );

