#include <X11/Xlib.h>
#include <stdlib.h>
#include <ctype.h>

#include "bind.h"

BindKeySymMask *keycode_convert( const char *binding_code )
{
  if (!binding_code) return 0;

  while ( *binding_code &&
      ( *binding_code==' ' || *binding_code=='\t' ) )
    binding_code++;

  if (!*binding_code) return 0;

  unsigned int mask = 0;

  while (*binding_code && *(binding_code+1) == '-')
  {
    switch ( tolower( *binding_code ) )
    {
      case 'c':
        mask += ControlMask;
        break;
      case 's':
        mask += ShiftMask;
        break;
      case 'a':
      case 'm':
        mask += Mod1Mask;
        break;
    }
    binding_code+=2;
  }

  if (!*binding_code) return 0;

  BindKeySymMask *binding = malloc( sizeof( BindKeySymMask ) );

  binding->mask = mask;
  binding->symbol = XStringToKeysym( binding_code );

  return binding;
}

