#include <stdlib.h>
#include <string.h>
#include <check.h>

#include "check_dictionary.h"
#include "dictionary.h"

START_TEST (test_dictionary_create)
{
  Dictionary *dictionary = dictionary_create();
  fail_unless( dictionary->data == NULL, NULL );
  fail_unless( dictionary->size == 0, NULL );
}
END_TEST

void *create_value(const char *string);
void *create_value(const char *string)
{
  char *value = malloc( strlen( string) + 1 );
  strcpy( value, string );
  return value;
}

START_TEST (test_dictionary_set_newValue)
{
  Dictionary *dictionary = dictionary_create();
  dictionary_set( dictionary, "foo", create_value("bar") );
  fail_unless( dictionary_haskey( dictionary, "foo" ),
      "Dictionary should have key" );
  fail_unless( dictionary_get( dictionary, "foo" ) != NULL,
      "Value should not be NULL" );
  fail_if( strcmp( dictionary_get( dictionary, "foo" ), "bar" ),
      "Value stored and value returned should be the same" );
  dictionary_destroy( dictionary );
}
END_TEST

START_TEST (test_dictionary_set_multipleNewValues)
{
  Dictionary *dictionary = dictionary_create();

  dictionary_set( dictionary, "foo", create_value("FOO") );
  dictionary_set( dictionary, "bar", create_value("BAR") );
  dictionary_set( dictionary, "baz", create_value("BAZ") );
  dictionary_set( dictionary, "foo.bar", create_value("FOO.BAR") );
  dictionary_set( dictionary, "foo.baz", create_value("FOO.BAZ") );

  fail_unless( dictionary_haskey( dictionary, "foo" ),
      "Dictionary should have key" );
  fail_unless( dictionary_get( dictionary, "foo" ) != NULL,
      "Value should not be NULL" );
  fail_if( strcmp( dictionary_get( dictionary, "foo" ), "FOO" ),
      "Value stored and value returned should be the same" );

  fail_unless( dictionary_haskey( dictionary, "bar" ),
      "Dictionary should have key" );
  fail_unless( dictionary_get( dictionary, "bar" ) != NULL,
      "Value should not be NULL" );
  fail_if( strcmp( dictionary_get( dictionary, "bar" ), "BAR" ),
      "Value stored and value returned should be the same" );

  fail_unless( dictionary_haskey( dictionary, "baz" ),
      "Dictionary should have key" );
  fail_unless( dictionary_get( dictionary, "baz" ) != NULL,
      "Value should not be NULL" );
  fail_if( strcmp( dictionary_get( dictionary, "baz" ), "BAZ" ),
      "Value stored and value returned should be the same" );

  fail_unless( dictionary_haskey( dictionary, "foo.bar" ),
      "Dictionary should have key" );
  fail_unless( dictionary_get( dictionary, "foo.bar" ) != NULL,
      "Value should not be NULL" );
  fail_if( strcmp( dictionary_get( dictionary, "foo.bar" ), "FOO.BAR" ),
      "Value stored and value returned should be the same" );

  fail_unless( dictionary_haskey( dictionary, "foo.baz" ),
      "Dictionary should have key" );
  fail_unless( dictionary_get( dictionary, "foo.baz" ) != NULL,
      "Value should not be NULL" );
  fail_if( strcmp( dictionary_get( dictionary, "foo.baz" ), "FOO.BAZ" ),
      "Value stored and value returned should be the same" );

  dictionary_destroy( dictionary );
}
END_TEST

START_TEST (test_dictionary_set_updateValue)
{
  Dictionary *dictionary = dictionary_create();
  dictionary_set( dictionary, "foo", create_value("bar") );
  dictionary_set( dictionary, "foo", create_value("baz") );
  fail_unless( dictionary_haskey( dictionary, "foo" ), NULL );
  fail_unless( !strcmp( dictionary_get( dictionary, "foo" ), "baz" ), NULL );
  dictionary_destroy( dictionary );
}
END_TEST

START_TEST (test_dictionary_set_setNull)
{
  Dictionary *dictionary = dictionary_create();
  dictionary_set( dictionary, "foo", create_value("bar") );
  dictionary_set( dictionary, "foo", NULL );
  fail_unless( !dictionary_haskey( dictionary, "foo" ), NULL );
  dictionary_destroy( dictionary );
}
END_TEST

START_TEST (test_dictionary_haskey_yes)
{
  Dictionary *dictionary = dictionary_create();
  dictionary_set( dictionary, "foo", create_value("bar") );
  fail_unless( dictionary_haskey( dictionary, "foo" ), NULL );
  dictionary_unset( dictionary, "foo" );
  dictionary_destroy( dictionary );
}
END_TEST

START_TEST (test_dictionary_haskey_no)
{
  Dictionary *dictionary = dictionary_create();
  fail_unless( !dictionary_haskey( dictionary, "foo" ), NULL );
  dictionary_destroy( dictionary );
}
END_TEST

START_TEST (test_dictionary_get_exists)
{
  Dictionary *dictionary = dictionary_create();
  dictionary_set( dictionary, "foo", create_value("bar") );
  fail_unless( !strcmp( dictionary_get( dictionary, "foo" ), "bar" ), NULL );
  dictionary_destroy( dictionary );
}
END_TEST

START_TEST (test_dictionary_get_notExists)
{
  Dictionary *dictionary = dictionary_create();
  fail_unless( dictionary_get( dictionary, "foo" ) == NULL, NULL );
  dictionary_destroy( dictionary );
}
END_TEST

START_TEST (test_dictionary_unset_exists)
{
  Dictionary *dictionary = dictionary_create();
  dictionary_set( dictionary, "foo", create_value("bar") );
  dictionary_unset( dictionary, "foo" );
  fail_unless( !dictionary_haskey( dictionary, "foo" ), NULL );
  dictionary_destroy( dictionary );
}
END_TEST

START_TEST (test_dictionary_unset_notExists)
{
  Dictionary *dictionary = dictionary_create();
  dictionary_unset( dictionary, "foo" );
  fail_unless( !dictionary_haskey( dictionary, "foo" ), NULL );
  dictionary_destroy( dictionary );
}
END_TEST

Suite *
dictionary_suite( void )
{
    Suite *suite = suite_create( "Dictionary" );

    TCase *tc_create    = tcase_create( "create" );
    TCase *tc_set     = tcase_create( "set" );
    TCase *tc_haskey  = tcase_create( "haskey" );
    TCase *tc_get     = tcase_create( "get" );
    TCase *tc_unset   = tcase_create( "unset" );

    suite_add_tcase( suite, tc_create );
    suite_add_tcase( suite, tc_set );
    suite_add_tcase( suite, tc_haskey );
    suite_add_tcase( suite, tc_get );
    suite_add_tcase( suite, tc_unset );

    tcase_add_test( tc_create, test_dictionary_create );

    tcase_add_test( tc_set, test_dictionary_set_newValue );
    tcase_add_test( tc_set, test_dictionary_set_multipleNewValues );
    tcase_add_test( tc_set, test_dictionary_set_updateValue );
    tcase_add_test( tc_set, test_dictionary_set_setNull );

    tcase_add_test( tc_haskey, test_dictionary_haskey_yes );
    tcase_add_test( tc_haskey, test_dictionary_haskey_no );

    tcase_add_test( tc_get, test_dictionary_get_exists );
    tcase_add_test( tc_get, test_dictionary_get_notExists );

    tcase_add_test( tc_unset, test_dictionary_unset_exists );
    tcase_add_test( tc_unset, test_dictionary_unset_notExists );

    return suite;
}
