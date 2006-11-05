#include <stdlib.h>
#include <string.h>
#include <check.h>

#include "check_dictionary.h"
#include "check_settings.h"

Suite * master_suite( void );

Suite *
master_suite( void )
{
    Suite *suite = suite_create( "Master" );
    return suite;
}

int main( void )
{
    int nf;

    SRunner *sr = srunner_create( master_suite() );

    srunner_add_suite( sr, dictionary_suite() );
    srunner_add_suite( sr, settings_suite() );

    srunner_run_all( sr, CK_ENV );
    nf = srunner_ntests_failed( sr );
    srunner_free( sr );
    return ( nf == 0 ) ? EXIT_SUCCESS : EXIT_FAILURE;

}
