/** Settings implementation.
 * \file settings.c
 */

#include <stdlib.h>
#include <string.h>

#include "dictionary.h"
#include "settings.h"
#include "commandline.h"

Dictionary *_settings;

static void settings_done( void );

void settings_init( void )
{
  _settings = dictionary_create();

  commandline_init();

  settings_set( "version", VERSION );

  settings_set( "border.colour.float.inactive", "grey50" );

  settings_set( "mouse.focus", "1" );

  settings_set( "text.delay", "1000" );

  settings_set( "text.colour.background", "Wheat" );
  settings_set( "text.colour.foreground", "Black" );

  settings_set( "text.border.colour", "goldenrod" );
  settings_set( "text.border.width", "1" );

  settings_set( "window.move.velocity", "16" );
  settings_set( "window.resize.velocity", "16" );

  commandline_add( "text.font",
      "fn", "font",
      "Font to use for any text displayed",
      "This is an X Server style font setting.\nYou can select any font to \
pass to this option by doing the following:\n\txfontsel -print\nUse the \
output generated from this as a parameter. It is probably best to enclose \
the font in quotes as some shells will attempt to expand the embedded `*'.",
      "-*-*-medium-r-*-*-14-*-*-*-*-*-*-*", 0 );

  commandline_add( "display",
      "d", "display",
      "X display to use",
      "This is the X display you wish to use. Normally in the format of `:0' \
or `:1.0'. The default empty value `' ensures that the current display is \
used.",
      "", 0 );

  commandline_add( "border.colour.active",
      "fg", "border-colour-active",
      "Active window's border colour.", 0,
      "goldenrod", 0 );

  commandline_add( "border.colour.inactive",
      "bg", "border-colour-inactive",
      "Inactive window's border colour.", 0,
      "grey50", 0 );

  commandline_add( "border.colour.float.active",
      "fc", "border-colour-float-active",
      "Active window's border colour when it's floating across desktops.", 0,
      "blue", 0 );
  commandline_add( "border.colour.float.inactive",
      "bc", "border-colour-float-inactive",
      "Inactive window's border colour when it's floating across desktops.", 0,
      "grey50", 0 );

  commandline_add( "border.width",
      "bw", "border-width",
      "Window border width", 0,
      "1", 0 );

  commandline_add( "prefix",
      "p", "prefix",
      "The command prefix keybinding", 0,
      "c-t", 0 );

  commandline_add( "mouse.warp",
      "mw", "mousewarp",
      "Should the mouse be \"warped\" to the active window when changing focus",
      0,
      "0", "1" );

  commandline_add( "border.snap",
      "snap", "border-snap",
      "Snap to window borders",
      "The value given defines, in pixels, the width of the snap",
      "0", 0 );

  commandline_add( "window.move.display",
      "nswm", "no-solid-window-move",
      "Don't display window contents when moving windows.", 0,
      "1", "0" );

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

  dictionary_set_free( _settings, key, newvalue );
}

unsigned int settings_haskey( const char *key )
{ return dictionary_haskey( _settings, key ); }

char *settings_get( const char *key )
{ return dictionary_get( _settings, key ); }

void settings_unset( const char *key )
{ dictionary_unset( _settings, key ); }
