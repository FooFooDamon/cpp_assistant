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
 * char_dictionary.cpp
 *
 *  Created on: 2015/1/22
 *      Author: wenxiongchang
 */

#include "char_dictionary.h"

#include "base/all.h"

#if HASH_OPTION == HASH_OPTION_STL_UNORDERED_MAP
#pragma message("unordered_map used for implementing dictionary")
#elif HASH_OPTION == HASH_OPTION_STL_MAP
#pragma message("map used for implementing dictionary")
#elif HASH_OPTION == HASH_OPTION_STL_HASH_MAP
#pragma message("hash_map used for implementing dictionary")
#else
#pragma message("dict from 3rd-party source code used for implementing dictionary")
#endif

#if(HASH_OPTION == HASH_OPTION_3RD_PARTY_DICT)

dict *char_dict_create(int dict_size)
{
    dict *d = NULL;

    if (NULL == (d = (dict *)dict_create(NULL)))
    {
        CA_ERROR("dict_create() failed\n");
        return NULL;
    }

    int actual_size = (dict_size < 0) ? DEFAULT_ELEMENT_COUNT : dict_size;

    // the bigger size, the lower possibility of collision
    dict_expand(d, actual_size);

    return d;
}

void char_dict_destroy(
    dict **dict,
    free_dict_value_func free_val_func,
    uint32_t free_hint
)
{
    if (NULL == dict || NULL == (*dict))
        return;

    dict_iterator *it = dict_get_safe_iterator(*dict);
    dict_entry *entry = NULL;

    while (NULL != (entry = dict_next(it)))
    {
        dict_delete_no_free(*dict, entry->key, entry->keylen);

        /*
         * release keys and values manually, do not depend on free() within dict functions
         */
        char_dict_release_key(&(entry->key));
        if (NULL != free_val_func)
            free_val_func(free_hint, &(entry->val));
    }

    dict_release_iterator(it);

    for (int i = 0; i <= 1; ++i)
    {
        if (NULL != (*dict)->ht[i].table)
        {
            free((*dict)->ht[i].table);
            (*dict)->ht[i].table = NULL;
        }
    }
    free(*dict);
    (*dict) = NULL;
}

int char_dict_add_element(
    const char *key,
    const int keylen,
    const void *val,
    const int vallen,
    dict *dict
)
{
    if (NULL == dict ||
        NULL == key ||
        NULL == val ||
        keylen <= 0 ||
        vallen <= 0)
    {
        CA_ERROR("invalid dict, key or value pointer, or invalid key or value length\n");
        return RET_FAILED;
    }

    char *_key = (char *)char_dict_alloc_key(keylen + 1); // one more byte for '\0'
    if (NULL == _key)
    {
        CA_ERROR("char_dict_alloc_key() failed\n");
        return RET_FAILED;
    }
    memcpy(_key, key, keylen);

    int ret = dict_add(dict, (void*)_key, keylen, (void*)val, vallen);

    if (DICT_OK != ret)
    {
        CA_ERROR("dict_add() failed\n");
        char_dict_release_key((void **)&_key);
        return RET_FAILED;
    }

    return RET_OK;
}

int char_dict_delete_element(
    const char *key,
    const int keylen,
    dict *dict,
    free_dict_value_func free_val_func,
    uint32_t free_hint
)
{
    if (NULL == key || NULL == dict)
    {
        CA_ERROR("null key or dict\n");
        return RET_FAILED;
    }

    dict_entry *entry = dict_find(dict, key, keylen);
    if (NULL == entry)
    {
        CA_ERROR("message with key[%s] not found\n", key);
        return RET_FAILED;
    }

    int ret = dict_delete_no_free(dict, entry->key, entry->keylen);

    char_dict_release_key(&(entry->key));
    if (NULL != free_val_func)
        free_val_func(free_hint, &(entry->val));

    return (DICT_OK == ret) ? RET_OK : RET_FAILED;
}

void *char_dict_find_iterator(const char *key, const int keylen, dict *dict)
{
    if (NULL == key || NULL == dict)
    {
        CA_ERROR("null key or dict\n");
        return NULL;
    }

    return dict_find(dict, key, keylen);
}

void *char_dict_find_value(const char *key, const int keylen, dict *dict)
{
    dict_entry *entry = (dict_entry*)char_dict_find_iterator(key, keylen, dict);

    if (NULL == entry)
        return NULL;

    return entry->val;
}

void char_dict_print(const dict *dict)
{
    dict_entry *entry = NULL;

    CA_INFO("Dictionary report:\n");
    CA_RAW_INFO("rehashidx = %d\n", dict->rehashidx);
    CA_RAW_INFO("iterators = %d\n", dict->iterators);
    for (int i = 0; i <= 1; ++i)
    {
        CA_RAW_INFO("ht[%d]:\n", i);
        CA_RAW_INFO("\tsize = %ld\n", dict->ht[i].size);
        CA_RAW_INFO("\tsizemask = 0x%lX\n", dict->ht[i].sizemask);
        CA_RAW_INFO("\tused = %ld\n", dict->ht[i].used);
        for (int j = 0; j < (int)dict->ht[i].size; ++j)
        {
            entry = dict->ht[i].table[j];
            CA_RAW_INFO("\ttable[%d]:\n", j);

            if (NULL == entry)
            {
                CA_RAW_INFO("\t\tNULL\n");
                continue;
            }

            while (NULL != entry)
            {
                CA_RAW_INFO("\t\t%s | %d | %p | %d\n", (char*)entry->key, entry->keylen, entry->val, entry->vallen);
                entry = entry->next;
            }
        }
    }
}

void char_dict_batch_operation(dict *dict, action_to_char_dict_element op)
{
    if (NULL == dict || NULL == op)
        return;

    dict_iterator *it = dict_get_safe_iterator(dict);
    dict_entry *entry = NULL;

    while (NULL != (entry = dict_next(it)))
    {
        op((char *)(entry->key), entry->val);
    }

    dict_release_iterator(it);
}

#else

dict *char_dict_create(int dict_size)
{
    return new dict; // TODO: how to use dict_size ?
}

void char_dict_destroy(
    dict **dict,
    free_dict_value_func free_val_func,
    uint32_t free_hint
)
{
    dict::iterator begin = (*dict)->begin();
    dict::iterator end = (*dict)->end();

    for (dict::iterator it = begin; it != end; ++it)
    {
//#if HASH_OPTION == HASH_OPTION_STL_MAP
#if 1
        char_dict_release_key((void **)&(it->first));
#endif
        if (NULL != free_val_func)
            free_val_func(free_hint, &(it->second));
    }

    (*dict)->clear();

    delete (*dict);
    (*dict) = NULL;
}

int char_dict_add_element(
    const char *key,
    const int keylen,
    const void *val,
    const int vallen,
    dict *dict
)
{
    if (NULL == dict ||
        NULL == key ||
        NULL == val ||
        keylen <= 0 ||
        vallen <= 0)
    {
        CA_ERROR("invalid dict, key or value pointer, or invalid key or value length\n");
        return RET_FAILED;
    }

    dict::iterator it = dict->find((char*)key);
    if (dict->end() != it)
    {
        CA_ERROR("message with key[%s] already existed\n", key);
        return RET_FAILED;
    }

    void *_val = (void*)val;
//#if HASH_OPTION == HASH_OPTION_STL_MAP
#if 1
    char *_key = (char *)char_dict_alloc_key(keylen + 1); // one more byte for '\0'

    if (NULL == _key)
    {
        CA_ERROR("calloc() for key failed\n");
        return RET_FAILED;
    }

    memcpy(_key, key, keylen);
#else
    char *_key = (char *)key;
#endif

    try
    {
        dict->insert(dict::value_type(_key, _val)).second;
    }
    catch(std::exception &e)
    {
        CA_ERROR("insert() failed: %s\n", e.what());
//#if HASH_OPTION == HASH_OPTION_STL_MAP
#if 1
        char_dict_release_key((void **)&_key);
#endif
        return RET_FAILED;
    }

    return RET_OK;
}

int char_dict_delete_element(
    const char *key,
    const int keylen,
    dict *dict,
    free_dict_value_func free_val_func,
    uint32_t free_hint
)
{
    if (NULL == key || NULL == dict)
    {
        CA_ERROR("null key or dict\n");
        return RET_FAILED;
    }

    dict::iterator it = dict->find((char*)key);
    if (dict->end() == it)
    {
        CA_ERROR("message with key[%s] not found\n", key);
        return RET_FAILED;
    }

//#if HASH_OPTION == HASH_OPTION_STL_MAP
#if 1
    char_dict_release_key((void **)&(it->first));
#endif
    if (NULL != free_val_func)
        free_val_func(free_hint, &(it->second));

    dict->erase(it);

    return RET_OK;
}

void *char_dict_find_iterator(const char *key, const int keylen, dict *dict)
{
    if (NULL == key || NULL == dict)
    {
        CA_ERROR("null key or dict\n");
        return NULL;
    }

    dict::iterator it = dict->find((char*)key);
    if (dict->end() == it)
        return NULL;

    return it;
}

void *char_dict_find_value(const char *key, const int keylen, dict *dict)
{
    dict::iterator it = (dict::iterator)char_dict_find_iterator(key, keylen, dict);

    if (dict->end() == it)
        return NULL;

    return it->second;
}

void char_dict_print(const dict *dict)
{
    dict::const_iterator begin = dict->begin();
    dict::const_iterator end = dict->end();
    int element_count = 0;

    CA_RAW_INFO_V("Dictionary report:\n");
    for (dict::const_iterator it = begin; it != end; ++it)
    {
        CA_RAW_INFO("element[%d]: %s | %d | %p | %d\n", element_count, (char*)it->first, -1, it->second, -1);
        ++element_count;
    }
}

void char_dict_batch_operation(dict *dict, action_to_char_dict_element op)
{
    ;
}

#endif
