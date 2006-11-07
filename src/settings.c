#include <stdlib.h>
#include <string.h>

#include "dictionary.h"
#include "settings.h"

Dictionary *_settings;

static void settings_done( void );

void settings_init( void )
{
  _settings = dictionary_create();
  /** \todo All default settings should be loaded here */

  settings_set( "border.width", "1" );

  settings_set( "border.colour.foreground", "goldenrod" );
  settings_set( "border.colour.background", "grey50" );

#ifdef  VWM
  settings_set( "border.colour.fixed", "blue" );
#endif

#ifdef SNAP
  settings_set( "border.snap", "0" );
#endif

  settings_set( "mouse.warp", "0" );

  settings_set( "text.delay", "1000" );

  atexit( settings_done );
}

static void settings_done( void )
{ dictionary_destroy( _settings ); }

void settings_set( const char *key, const char *value )
{
  char *newvalue;
  if ( value )
  {
    newvalue = malloc( strlen( value ) + 1 );
    strcpy( newvalue, value );
  }
  else
    newvalue = 0;

  dictionary_set( _settings, key, newvalue );
}

unsigned int settings_haskey( const char *key )
{ return dictionary_haskey( _settings, key ); }

char *settings_get( const char *key )
{ return dictionary_get( _settings, key ); }

void settings_unset( const char *key )
{ dictionary_unset( _settings, key ); }
