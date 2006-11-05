#include <stdlib.h>
#include <string.h>

#include "dictionary.h"

Dictionary *dictionary_create( void )
{
  Dictionary *dictionary = malloc( sizeof( Dictionary ) );
  dictionary->data=0;
  dictionary->size=0;
  return dictionary;
}

void dictionary_destroy( Dictionary *dictionary )
{
  while ( dictionary->size )
    dictionary_unset( dictionary, ((DictionaryPair *)dictionary->data)->key );
  free( dictionary );
}

static DictionaryPair *dictionary_find(
    Dictionary *dictionary, const char * key )
{
  /** \todo This must be rewritten to do a binary search after. */
  unsigned int key_enum;
  for ( key_enum = 0; key_enum < dictionary->size; key_enum++ )
    if (!strcmp( key, dictionary->data[ key_enum ].key ) )
      return &dictionary->data[ key_enum ];

  return 0;
}

static void dictionary_remove(
    Dictionary *dictionary, const unsigned int key_enum )
{
  free( dictionary->data[ key_enum ].key );
  free( dictionary->data[ key_enum ].value );

  if ( key_enum < dictionary->size-1 )
    memmove(
        &dictionary->data[ key_enum ],
        &dictionary->data[ key_enum + 1],
        sizeof( DictionaryPair )*( ( dictionary->size - 1 ) - key_enum )
        );

  dictionary->size--;
  dictionary->data =
    realloc( dictionary->data, sizeof( DictionaryPair ) * dictionary->size );

}

void dictionary_set(
    Dictionary *dictionary, const char *key, const void *value )
{
  DictionaryPair *pair = dictionary_find( dictionary, key );

  if ( !value )
  {
    if ( !pair ) return;

    unsigned int key_enum;
    for ( key_enum = 0; key_enum < dictionary->size; key_enum++ )
      if ( pair == &dictionary->data[ key_enum ] )
      {
        dictionary_remove( dictionary, key_enum );
        break;
      }

    return;
  }

  if ( !pair )
  {
    /** \todo This must be rewritten to do a binary sorted-insert. */
    if ( dictionary->size )
      dictionary->data =
        realloc(
            dictionary->data, sizeof( DictionaryPair ) * ( dictionary->size + 1 ) );
    else
      dictionary->data = malloc( sizeof( DictionaryPair ) );

    dictionary->size++;
    pair = dictionary->data;

    pair->key = malloc( strlen( key ) + 1 );
    strcpy( pair->key, key );

  } else {
    free( pair->value );
  }

  pair->value = value;
}

unsigned int dictionary_haskey( Dictionary *dictionary, const char *key )
{ return dictionary_get( dictionary, key ) != 0; }

void *dictionary_get( Dictionary *dictionary, const char *key )
{
  DictionaryPair *pair = dictionary_find( dictionary, key );
  return pair ? pair->value : 0;
}

void dictionary_unset( Dictionary *dictionary, const char *key )
{ dictionary_set( dictionary, key, 0 ); }

