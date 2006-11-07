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

  settings_set( "border.colour.active", "goldenrod" );
  settings_set( "border.colour.inactive", "grey50" );

#ifdef  VWM
  settings_set( "border.colour.fixed.active", "blue" );
  settings_set( "border.colour.fixed.inactive", "grey50" );
#endif

#ifdef SNAP
  settings_set( "border.snap", "0" );
#endif

  settings_set( "mouse.warp", "0" );

  settings_set( "text.delay", "1000" );

  settings_set( "text.colour.background", "Wheat" );
  settings_set( "text.border.colour", "goldenrod" );
  settings_set( "text.border.width", "1" );

  settings_set( "text.font", "-*-*-medium-r-*-*-14-*-*-*-*-*-*-*" );

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
