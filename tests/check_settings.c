#include <stdlib.h>
#include <string.h>
#include <check.h>

#include "check_settings.h"
#include "settings.h"

START_TEST (test_settings_set_newValue)
{
  settings_set( "foo", "bar" );
  fail_unless( settings_haskey( "foo" ), NULL );
  fail_unless( !strcmp( settings_get( "foo" ), "bar" ), NULL );
  settings_unset( "foo" );
}
END_TEST

START_TEST (test_settings_set_updateValue)
{
  settings_set( "foo", "bar" );
  settings_set( "foo", "baz" );
  fail_unless( settings_haskey( "foo" ), NULL );
  fail_unless( !strcmp( settings_get( "foo" ), "baz" ), NULL );
  settings_unset( "foo" );
}
END_TEST

START_TEST (test_settings_set_setNull)
{
  settings_set( "foo", "bar" );
  settings_set( "foo", NULL );
  fail_unless( !settings_haskey( "foo" ), NULL );
}
END_TEST

START_TEST (test_settings_haskey_yes)
{
  settings_set( "foo", "bar" );
  fail_unless( settings_haskey( "foo" ), NULL );
  settings_unset( "foo" );
}
END_TEST

START_TEST (test_settings_haskey_no)
{
  settings_unset( "foo" );
  fail_unless( !settings_haskey( "foo" ), NULL );
}
END_TEST

START_TEST (test_settings_get_exists)
{
  settings_set( "foo", "bar" );
  fail_unless( !strcmp( settings_get( "foo" ), "bar" ), NULL );
  settings_unset( "foo" );
}
END_TEST

START_TEST (test_settings_get_notExists)
{
  settings_unset( "foo" );
  fail_unless( settings_get( "foo" ) == NULL, NULL );
}
END_TEST

START_TEST (test_settings_unset_exists)
{
  settings_set( "foo", "bar" );
  settings_unset( "foo" );
  fail_unless( !settings_haskey( "foo" ), NULL );
}
END_TEST

START_TEST (test_settings_unset_notExists)
{
  settings_unset( "foo" );
  fail_unless( !settings_haskey( "foo" ), NULL );
  settings_unset( "foo" );
  fail_unless( !settings_haskey( "foo" ), NULL );
}
END_TEST

Suite *
settings_suite( void )
{
    Suite *suite = suite_create( "Settings" );
    
    settings_init();

    TCase *tc_set     = tcase_create( "set" );
    TCase *tc_haskey  = tcase_create( "haskey" );
    TCase *tc_get     = tcase_create( "get" );
    TCase *tc_unset   = tcase_create( "unset" );

    suite_add_tcase( suite, tc_set );
    suite_add_tcase( suite, tc_haskey );
    suite_add_tcase( suite, tc_get );
    suite_add_tcase( suite, tc_unset );

    tcase_add_test( tc_set, test_settings_set_newValue );
    tcase_add_test( tc_set, test_settings_set_updateValue );
    tcase_add_test( tc_set, test_settings_set_setNull );

    tcase_add_test( tc_haskey, test_settings_haskey_yes );
    tcase_add_test( tc_haskey, test_settings_haskey_no );

    tcase_add_test( tc_get, test_settings_get_exists );
    tcase_add_test( tc_get, test_settings_get_notExists );

    tcase_add_test( tc_unset, test_settings_unset_exists );
    tcase_add_test( tc_unset, test_settings_unset_notExists );

    return suite;
}
