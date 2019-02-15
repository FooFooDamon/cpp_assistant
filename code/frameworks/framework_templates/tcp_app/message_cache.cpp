/*
 * Copyright (c) 2016-2019, Wen Xiongchang <udc577 at 126 dot com>
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

#include "message_cache.h"

#include "protocol_common.h"
#include "config_manager.h"

namespace cafw
{

message_cache::message_cache()
    : m_message_dictionary(NULL)
{
    ;
}

message_cache::message_cache(int dict_size)
    : m_message_dictionary(NULL)
{
    create(dict_size);
}

message_cache::~message_cache()
{
    destroy();
}

int message_cache::create(int dict_size)
{
    if ((NULL == m_message_dictionary) &&
        (NULL == (m_message_dictionary = char_dict_create(DEFAULT_DICT_SIZE))))
    {
        LOGF_C(E, "char_dict_create() failed\n");
        return RET_FAILED;
    }

    return RET_OK;
}

static void __free_message_value(uint32_t unused_hint, void **val)
{
    if (NULL == val || NULL == (*val))
        return;

    msg_cache_value *whole_msg = (msg_cache_value *)(*val);
    msg_base *msg_body = (msg_base *)(whole_msg->contents);

    char_dict_release_val((msg_base **)&msg_body);
    char_dict_release_val((msg_cache_value **)val);
}

void message_cache::destroy(void)
{
    char_dict_destroy(&m_message_dictionary, __free_message_value, NO_FREE_HINT);
}

int message_cache::add(const char *key, const int keylen, const void *val, const int vallen)
{
    return char_dict_add_element(key, keylen, val, vallen, m_message_dictionary);
}

int message_cache::del(const char *key, const int keylen)
{
    return char_dict_delete_element(key, keylen, m_message_dictionary, __free_message_value, NO_FREE_HINT);
}

void* message_cache::find(const char *key, const int keylen)
{
    return char_dict_find_value(key, keylen, m_message_dictionary);
}

#if HASH_OPTION == HASH_OPTION_3RD_PARTY_DICT

void message_cache::clean_expired_messages(int64_t cur_utc_usec)
{
    dict_iterator *it = dict_get_iterator(m_message_dictionary);
    dict_entry *entry = NULL;
    int del_count = 0;
    int64_t msg_time = 0;
    msg_cache_value *msg_item = NULL;
    static const int64_t kDefaultTimeout = CFG_GET_TIMEOUT_USEC(XNODE_DEFAULT_MSG_PROCESS);
    static const int64_t kMaxTimeout = CFG_GET_TIMEOUT_USEC(XNODE_MAX_MSG_PROCESS);

    while (NULL != (entry = dict_next(it)))
    {
        if (NULL == (msg_item = (msg_cache_value *)(entry->val)))
            continue;

        msg_time = msg_item->last_op_time;

        bool msg_is_time_consuming = message_is_time_consuming(msg_item->cmd);
        bool msg_expired = msg_is_time_consuming
            ? (cur_utc_usec - msg_time >= kMaxTimeout)
            : (cur_utc_usec - msg_time >= kDefaultTimeout);

        if (msg_expired)
        {
            LOGF_C(I, "message[%s | 0x%08X] last operated on time[%ld] expired, cleaned up now\n",
                (char*)(entry->key), msg_item->reqcmd, msg_time);
            dict_delete_no_free(m_message_dictionary, entry->key, entry->keylen);
            char_dict_release_key((void **)&(entry->key));
            __free_message_value(NO_FREE_HINT, &(entry->val));
            ++del_count;
        }
    }

    dict_release_iterator(it);

    if (del_count > 0)
        RLOGF(I, "%d expired messages cleaned up\n", del_count);
}

size_t message_cache::time_consuming_message_count(const char *connection_name) const
{
    if (NULL == connection_name)
        return 0;

    dict_iterator *it = dict_get_iterator(m_message_dictionary);
    dict_entry *entry = NULL;
    int target_count = 0;
    msg_cache_value *msg_item = NULL;

    while (NULL != (entry = dict_next(it)))
    {
        if (NULL == (msg_item = (msg_cache_value *)(entry->val)) || NULL == msg_item->to_name)
            continue;

        bool msg_is_time_consuming = message_is_time_consuming(msg_item->cmd);

        if (!msg_is_time_consuming)
            continue;

        if (0 == strcmp(connection_name, msg_item->to_name))
            ++target_count;
    }

    dict_release_iterator(it);

    return target_count;
}

#else

void message_cache::clean_expired_messages(int64_t cur_utc_usec)  // TODO: to be improved ...
{
    Dictionary::iterator begin = m_message_dictionary->begin();
    Dictionary::iterator end = m_message_dictionary->end();
    Dictionary::iterator tmp_it = m_message_dictionary->end();
    int del_count = 0;
    int64_t msg_time = 0;
    static const int64_t kDefaultTimeout = CFG_GET_TIMEOUT_USEC(XNODE_DEFAULT_MSG_PROCESS);
    static const int64_t kMaxTimeout = CFG_GET_TIMEOUT_USEC(XNODE_MAX_MSG_PROCESS);

    for (Dictionary::iterator it = begin; it != end;)
    {
        tmp_it = it++;

        if (NULL == tmp_it->second)
            continue;

        msg_time = ((msg_cache_value *)(tmp_it->second))->last_op_time;

        bool msg_is_time_consuming = message_is_time_consuming(msg_item->cmd);
        bool msg_expired = msg_is_time_consuming
            ? (cur_utc_usec - msg_time >= kMaxTimeout)
            : (cur_utc_usec - msg_time >= kDefaultTimeout);

        if (msg_expired)
        {
            LOG_INFO_CV("message[%s] last operated on time[%lld] expired, cleaned up now\n", tmp_it->first, msg_time);
//#if HASH_OPTION == HASH_OPTION_STL_MAP
#if 1
            char_dict_release_key((void **)&(tmp_it->first));
#endif
            __free_message_value(NO_FREE_HINT, &(tmp_it->second));
            m_message_dictionary->erase(tmp_it);
            ++del_count;
        }
    }

    if (del_count > 0)
        LOG_INFO("%d expired messages cleaned up\n", del_count);
}

size_t message_cache::time_consuming_message_count(const char *connection_name) const
{
    // TODO: implement it when it is needed
}

#endif

void message_cache::print(void)
{
    char_dict_print(m_message_dictionary);
}

}
