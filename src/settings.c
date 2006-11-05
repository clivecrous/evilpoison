#include <stdlib.h>
#include <string.h>

#include "settings.h"

typedef struct {
  char *key;
  char *value;
} KeyValue;

KeyValue *_settings;
unsigned int _settings_amount = 0;

void settings_init( void )
{
  /** \todo All default settings should be loaded here */
}

static KeyValue *settings_find( const char * key )
{
  /** \todo This must be rewritten to do a binary search after. */
  unsigned int key_enum;
  for ( key_enum = 0; key_enum < _settings_amount; key_enum++ )
    if (!strcmp( key, _settings[ key_enum ].key ) ) return &_settings[ key_enum ];

  return 0;
}

static void settings_remove( const unsigned int key_enum )
{
  free( _settings[ key_enum ].key );
  free( _settings[ key_enum ].value );

  if ( key_enum < _settings_amount-1 )
    memmove(
        &_settings[ key_enum ],
        &_settings[ key_enum + 1],
        sizeof( KeyValue )*( ( _settings_amount - 1 ) - key_enum )
        );

  _settings_amount--;
  _settings = realloc( _settings, sizeof( KeyValue ) * _settings_amount );

}

void settings_set( const char *key, const char *value )
{
  KeyValue *setting = settings_find( key );

  if ( !value )
  {
    if ( !setting ) return;

    unsigned int key_enum;
    for ( key_enum = 0; key_enum < _settings_amount; key_enum++ )
      if ( setting == &_settings[ key_enum ] )
      {
        settings_remove( key_enum );
        break;
      }

    return;
  }

  if ( !setting )
  {
    /** \todo This must be rewritten to do a binary sorted-insert. */
    if ( _settings_amount )
      _settings =
        realloc( _settings, sizeof( KeyValue ) * ( _settings_amount + 1 ) );
    else
      _settings = malloc( sizeof( KeyValue ) );

    _settings_amount++;
    setting = _settings;

    setting->key = malloc( strlen( key ) + 1 );
    strcpy( setting->key, key );

  } else {
    free( setting->value );
  }

  setting->value = malloc( strlen( value ) + 1 );
  strcpy( setting->value, value );

}

unsigned int settings_haskey( const char *key )
{ return settings_get( key ) != 0; }

char *settings_get( const char *key )
{
  KeyValue *keyvalue = settings_find( key );
  return keyvalue ? keyvalue->value : 0;
}

void settings_unset( const char *key )
{ settings_set( key, 0 ); }
