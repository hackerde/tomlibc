#ifndef __LIB_KEY_H__
#define __LIB_KEY_H__

#include "models.h"

key_t*  new_key         ( key_type_t type );

key_t*  has_subkey      ( key_t* key, key_t* subkey );
key_t*  add_subkey      ( key_t* key, key_t* subkey );
key_t*  get_key         ( key_t* key, const char* id );

bool    compatible_keys ( key_type_t existing, key_type_t current );

void    delete_key      ( key_t* key );

#endif
