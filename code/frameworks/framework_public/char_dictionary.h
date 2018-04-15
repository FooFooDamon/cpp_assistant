/*
 * Copyright (c) 2016-2018, Wen Xiongchang <udc577 at 126 dot com>
 * All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must
 * not claim that you wrote the original software. If you use this
 * software in a product, an acknowledgment in the product documentation
 * would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 */

// NOTE: The original author also uses (short/code) names listed below,
//       for convenience or for a certain purpose, at different places:
//       wenxiongchang, wxc, Damon Wen, udc577

/*
 * char_dictionary.h
 *
 *  Created on: 2015/1/22
 *      Author: wenxiongchang
 */

#ifndef __CASDK_FRAMEWORK_CHAR_DICTIONARY_H__
#define __CASDK_FRAMEWORK_CHAR_DICTIONARY_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cpp_assistant/ca_full.h>

#if 0
enum
{
    HASH_OPTION_STL_UNORDERED_MAP = 1,
    HASH_OPTION_STL_MAP,
    HASH_OPTION_STL_HASH_MAP,
    HASH_OPTION_3RD_PARTY_DICT
};
#else // Enable this branch when macro compare is used.
#define HASH_OPTION_STL_UNORDERED_MAP                               1
#define HASH_OPTION_STL_MAP                                         2
#define HASH_OPTION_STL_HASH_MAP                                    3
#define HASH_OPTION_3RD_PARTY_DICT                                  4
#endif

#ifndef HASH_OPTION
#define HASH_OPTION                                                 HASH_OPTION_3RD_PARTY_DICT
#endif

#if HASH_OPTION == HASH_OPTION_STL_UNORDERED_MAP
#include <unordered_map>
typedef std::unordered_map<char*, void*, std::hash<char*>, cal::char_eq_op > dict;
#elif HASH_OPTION == HASH_OPTION_STL_MAP
#include <map>
typedef std::map<char*, void*, cal::char_lt_op > dict;
#elif HASH_OPTION == HASH_OPTION_STL_HASH_MAP
#include <hash_map>
typedef __gnu_cxx::hash_map<char*, void*, __gnu_cxx::hash<char*>, cal::char_eq_op > dict;
#else
//typedef dict dict;
#endif

#if HASH_OPTION == HASH_OPTION_3RD_PARTY_DICT
typedef dict_entry* dict_entry_ptr;
#define GET_CHAR_DICT_KEY(dict_item)    ((char*)((dict_item)->key))
#define GET_CHAR_DICT_VALUE(dict_item)  ((dict_item)->val)
#else
typedef dict::iterator dict_entry_ptr;
#define GET_CHAR_DICT_KEY(dict_item)    ((char*)((dict_item)->first))
#define GET_CHAR_DICT_VALUE(dict_item)  ((dict_item)->second)
#endif

enum
{
    DEFAULT_ELEMENT_COUNT = 1024
};

typedef void (*action_to_char_dict_element)(char *key, void *val);

inline void *char_dict_alloc_key(const int keylen)
{
    return calloc(keylen, sizeof(char));
}

inline void char_dict_release_key(void **key)
{
    if ((NULL != key) && (NULL != (*key)))
    {
        free(*key);
        (*key) = NULL;
    }
}

template < typename T >
inline T *char_dict_alloc_val(const T *src = NULL)
{
    if (NULL == src)
        return new T;
    else
        return new T(*src);
}

template < typename T >
inline void char_dict_release_val(T **val)
{
    if ((NULL != val) && (NULL != (*val)))
    {
        delete (T *)(*val);
        (*val) = NULL;
    }
}

typedef void (*free_dict_value_func)(uint32_t hint, void **val);

enum
{
    NO_FREE_HINT = 0
};

dict *char_dict_create(int dict_size);

void char_dict_destroy(
    dict **dict,
    free_dict_value_func free_val_func = NULL,
    uint32_t free_hint = NO_FREE_HINT
);

int char_dict_add_element(
    const char *key,
    const int keylen,
    const void *val,
    const int vallen,
    dict *dict
);

int char_dict_delete_element(
    const char *key,
    const int keylen,
    dict *dict,
    free_dict_value_func free_val_func = NULL,
    uint32_t free_hint = NO_FREE_HINT
);

void *char_dict_find_iterator(const char *key, const int keylen, dict *dict);

void *char_dict_find_value(const char *key, const int keylen, dict *dict);

void char_dict_print(const dict *dict);

void char_dict_batch_operation(dict *dict, action_to_char_dict_element op);

#endif /* __CASDK_FRAMEWORK_CHAR_DICTIONARY_H__ */
