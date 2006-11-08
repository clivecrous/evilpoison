#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <check.h>

#include "check_bind.h"
#include "bind.h"

START_TEST ( test_bind_keycode_convert_null )
{
  fail_unless(
      keycode_convert( NULL ) == NULL,
      "Passing NULL should return NULL" );
}
END_TEST

START_TEST ( test_bind_keycode_convert_emptyString )
{
  fail_unless(
      keycode_convert( "" ) == NULL,
      "Passing an empty string should return NULL" );
}
END_TEST

START_TEST ( test_bind_keycode_convert_justMods )
{
  fail_unless(
      keycode_convert( "s-" ) == NULL,
      "Passing shift alone should return NULL" );
  fail_unless(
      keycode_convert( "c-" ) == NULL,
      "Passing control alone should return NULL" );
  fail_unless(
      keycode_convert( "a-" ) == NULL,
      "Passing alt alone should return NULL" );
  fail_unless(
      keycode_convert( "@-" ) == NULL,
      "Passing an unknown modifier alone should return NULL" );
  fail_unless(
      keycode_convert( "s-c-a-c-s-a-a-s-@-" ) == NULL,
      "Passing only modifiers should return NULL" );
}
END_TEST

START_TEST ( test_bind_keycode_convert_ModMasks )
{
  BindKeySymMask *binding;

  binding = keycode_convert( "s-x" );
  fail_unless(
      binding->mask == ShiftMask,
      "Incorrect interpretation of 's-x':'s-'" );
  fail_unless(
      binding->symbol == XK_x,
      "Incorrect interpretation of 's-x':'-x'" );

  binding = keycode_convert( "a-x" );
  fail_unless(
      binding->mask == Mod1Mask,
      "Incorrect interpretation of 'a-x':'a-'" );
  fail_unless(
      binding->symbol == XK_x,
      "Incorrect interpretation of 'a-x':'-x'" );

  binding = keycode_convert( "m-x" );
  fail_unless(
      binding->mask == Mod1Mask,
      "Incorrect interpretation of 'm-x':'m-'" );
  fail_unless(
      binding->symbol == XK_x,
      "Incorrect interpretation of 'm-x':'-x'" );

  binding = keycode_convert( "c-x" );
  fail_unless(
      binding->mask == ControlMask,
      "Incorrect interpretation of 'c-x':'c-'" );
  fail_unless(
      binding->symbol == XK_x,
      "Incorrect interpretation of 'c-x':'-x'" );

  binding = keycode_convert( "@-x" );
  fail_unless(
      binding->mask == 0,
      "Incorrect interpretation of '@-x':'@-'" );
  fail_unless(
      binding->symbol == XK_x,
      "Incorrect interpretation of '@-x':'-x'" );

  binding = keycode_convert( "s-c-a-x" );
  fail_unless(
      binding->mask == (Mod1Mask|ShiftMask|ControlMask),
      "Incorrect interpretation of 's-c-a-x':'s-c-a-'" );
  fail_unless(
      binding->symbol == XK_x,
      "Incorrect interpretation of 's-c-a-x':'-x'" );

  binding = keycode_convert( "x" );
  fail_unless(
      binding->mask == 0,
      "Incorrect interpretation of 'x':''" );
  fail_unless(
      binding->symbol == XK_x,
      "Incorrect interpretation of 'x':'x'" );

}
END_TEST


Suite *
bind_suite( void )
{
    Suite *suite = suite_create( "bind" );
    
    TCase *tc_keycode_convert   = tcase_create( "keycode_convert" );

    suite_add_tcase( suite, tc_keycode_convert );

    tcase_add_test( tc_keycode_convert, test_bind_keycode_convert_null );
    tcase_add_test( tc_keycode_convert, test_bind_keycode_convert_emptyString );
    tcase_add_test( tc_keycode_convert, test_bind_keycode_convert_justMods );
    tcase_add_test( tc_keycode_convert, test_bind_keycode_convert_ModMasks );

    return suite;
}
