#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <check.h>

#include "check_command.h"
#include "command.h"
#include "settings.h"

char *greet(char *whom);
char *greet(char *whom)
{
  char *greeting = malloc( strlen( whom) + 8);
  sprintf( greeting, "Hello %s!", whom );
  return greeting;
}

char *set(char *commandline);
char *set(char *commandline)
{
  char *newcommandline = malloc( strlen( commandline ) + 1 );
  strcpy( newcommandline, commandline );

  char *setting = newcommandline;
  while (*setting && *setting==' ' ) setting++;

  char *value = setting;
  while (*value && !isblank(*value) ) value++;
  while (*value && isblank(*value) ) { *value='\0'; value++; };

  settings_set( setting, value );

  free( newcommandline );

  return 0;
}

/* ---------------------------------------------------------------------- */

START_TEST ( test_command_assign_null )
{
  command_assign( "foo", NULL );
  command_unassign( "foo" );
}
END_TEST

START_TEST ( test_command_assign_greet )
{
  command_assign( "foo", greet );
  command_unassign( "foo" );
}
END_TEST

START_TEST ( test_command_execute_null )
{
  command_unassign( "greet" );
  fail_unless(
      command_execute( "greet World" ) == NULL,
      "Greeting return value should be NULL" );
}
END_TEST

START_TEST ( test_command_execute_helloWorld )
{
  command_assign( "greet", greet );
  fail_unless(
      !strcmp( command_execute( "greet World" ), "Hello World!" ),
      "Greeting return value is not correct." );
  command_unassign( "greet" );
}
END_TEST

START_TEST ( test_command_execute_doubleDollar )
{
  command_assign( "greet", greet );
  fail_unless(
      !strcmp( command_execute( "greet $$" ), "Hello !" ),
      "Greeting return value is not correct." );
  command_unassign( "greet" );
}
END_TEST

START_TEST ( test_command_execute_escapedDollar )
{
  command_assign( "greet", greet );
  fail_unless(
      !strcmp( command_execute( "greet \\$" ), "Hello $!" ),
      "Greeting return value is not correct." );
  command_unassign( "greet" );
}
END_TEST

START_TEST ( test_command_execute_escapedDoubleDollar )
{
  command_assign( "greet", greet );
  fail_unless(
      !strcmp( command_execute( "greet \\$\\$" ), "Hello $$!" ),
      "Greeting return value is not correct." );
  command_unassign( "greet" );
}
END_TEST

START_TEST ( test_command_execute_dollarFooDollar )
{
  settings_set( "foo", "bar" );
  command_assign( "greet", greet );
  fail_unless(
      !strcmp( command_execute( "greet $foo$" ), "Hello bar!" ),
      "Greeting return value is not correct." );
  command_unassign( "greet" );
  settings_unset( "foo" );
}
END_TEST

START_TEST ( test_command_execute_set )
{

  settings_set( "foo", "bar" );
  command_assign( "set", set );
  command_execute( "set foo baz" );
  fail_unless( !strcmp( settings_get( "foo" ), "baz" ), NULL );
  command_unassign( "set" );
  settings_unset( "foo" );
}
END_TEST

START_TEST ( test_command_execute_twoCommands )
{
  settings_set( "foo", "bar" );
  command_assign( "greet", greet );
  command_assign( "set", set );
  fail_unless(
      !strcmp( command_execute( "set foo baz;greet $foo$" ), "Hello baz!" ),
      "Greeting return value is not correct." );
  command_unassign( "set" );
  command_unassign( "greet" );
  settings_unset( "foo" );
}
END_TEST

START_TEST ( test_command_alias_exists )
{
  command_assign( "greet", greet );
  alias_assign( "foo", "greet World" );
  fail_unless(
      !strcmp( command_execute( "foo" ), "Hello World!" ),
      "Greeting return value is not correct." );
  alias_unassign( "foo" );
  command_unassign( "greet" );
}
END_TEST

START_TEST ( test_command_parameter_copy_whichOneOfOne )
{
  char * result = command_parameter_copy( "One", 0 );
  fail_unless(
      !strcmp( result, "One" ),
      "Incorrect parameter result." );
  free( result );
}
END_TEST

START_TEST ( test_command_parameter_copy_whichOneOfNone )
{
  char * result = command_parameter_copy( "", 0 );
  fail_unless(
      result == NULL,
      "Result should fail" );
}
END_TEST

START_TEST ( test_command_parameter_copy_whichTwoOfOne )
{
  char * result = command_parameter_copy( "One", 1 );
  fail_unless(
      result == NULL,
      "Result should fail" );
}
END_TEST

START_TEST ( test_command_parameter_copy_whichOneOfTwo )
{
  char * result = command_parameter_copy( "One Two", 0 );
  fail_unless(
      !strcmp( result, "One" ),
      "Incorrect parameter result." );
  free( result );
}
END_TEST

START_TEST ( test_command_parameter_copy_whichTwoOfThree )
{
  char * result = command_parameter_copy( "One Two Three", 1 );
  fail_unless(
      !strcmp( result, "Two" ),
      "Incorrect parameter result." );
  free( result );
}
END_TEST

START_TEST ( test_command_parameter_copy_whichTwoOfTwo )
{
  char * result = command_parameter_copy( "One Two", 1 );
  fail_unless(
      !strcmp( result, "Two" ),
      "Incorrect parameter result." );
  free( result );
}
END_TEST

START_TEST ( test_command_parameter_copy_NullParameters )
{
  char * result = command_parameter_copy( NULL, 0 );
  fail_unless(
      result == NULL,
      "Result should fail" );
}
END_TEST

Suite *
command_suite( void )
{
    Suite *suite = suite_create( "Command" );
    
    command_init();

    TCase *tc_assign   = tcase_create( "assign" );
    TCase *tc_unassign = tcase_create( "unassign" );
    TCase *tc_execute  = tcase_create( "execute" );
    TCase *tc_alias  = tcase_create( "alias" );
    TCase *tc_command_parameter  = tcase_create( "command_parameter" );

    suite_add_tcase( suite, tc_assign );
    suite_add_tcase( suite, tc_unassign );
    suite_add_tcase( suite, tc_execute );
    suite_add_tcase( suite, tc_alias );
    suite_add_tcase( suite, tc_command_parameter );

    tcase_add_test( tc_assign, test_command_assign_null );
    tcase_add_test( tc_assign, test_command_assign_greet );

    tcase_add_test( tc_assign, test_command_execute_null );
    tcase_add_test( tc_assign, test_command_execute_helloWorld );
    tcase_add_test( tc_assign, test_command_execute_doubleDollar );
    tcase_add_test( tc_assign, test_command_execute_escapedDollar );
    tcase_add_test( tc_assign, test_command_execute_escapedDoubleDollar );
    tcase_add_test( tc_assign, test_command_execute_dollarFooDollar );
    tcase_add_test( tc_assign, test_command_execute_set );
    tcase_add_test( tc_assign, test_command_execute_twoCommands );

    tcase_add_test( tc_alias, test_command_alias_exists );

    tcase_add_test( tc_command_parameter,
        test_command_parameter_copy_whichOneOfOne );
    tcase_add_test( tc_command_parameter,
        test_command_parameter_copy_whichOneOfNone );
    tcase_add_test( tc_command_parameter,
        test_command_parameter_copy_whichTwoOfOne );
    tcase_add_test( tc_command_parameter,
        test_command_parameter_copy_whichOneOfTwo );
    tcase_add_test( tc_command_parameter,
        test_command_parameter_copy_whichTwoOfThree );
    tcase_add_test( tc_command_parameter,
        test_command_parameter_copy_whichTwoOfTwo );
    tcase_add_test( tc_command_parameter,
        test_command_parameter_copy_NullParameters );

    return suite;
}
