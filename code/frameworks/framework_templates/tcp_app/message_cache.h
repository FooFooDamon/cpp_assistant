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
 * message_cache.h
 *
 *  Created on: 2015-06-02
 *      Author: wenxiongchang
 * Description: for caching pending messages
 */

#ifndef __MESSAGE_CACHE_H__
#define __MESSAGE_CACHE_H__

#include <stdint.h>
#include <stddef.h>

#include "char_dictionary.h"

namespace cafw
{

typedef struct msg_cache_value
{
    union
    {
        uint32_t reqcmd;
        uint32_t respcmd;
        uint32_t cmd;
    };
    //union
    //{
        int from_fd;
        char *from_name;
    //};
    //union
    //{
        int to_fd;
        char *to_name;
    //};
    int64_t last_op_time;
    void *contents;
}msg_cache_value;

class message_cache
{
/* ===================================
 * constructors:
 * =================================== */
public:
    message_cache();
    message_cache(int dict_size);

/* ===================================
 * copy control:
 * =================================== */
private:
    message_cache(const message_cache& src);
    message_cache& operator=(const message_cache& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~message_cache();

/* ===================================
 * types:
 * =================================== */
public:
    enum enum_dict_size
    {
        DEFAULT_DICT_SIZE = 1024
    };
    enum
    {
        MAX_MSG_EXPIRED_USECS = 5 * 60 * 1000000
    };

/* ===================================
 * abilities:
 * =================================== */
public:
    int create(int dict_size = DEFAULT_DICT_SIZE);
    void destroy(void);
    int add(const char *key, const int keylen, const void *val, const int vallen);
    int del(const char *key, const int keylen);
    void *find(const char *key, const int keylen);
    void clean_expired_messages(int64_t cur_utc_usec);
    size_t time_consuming_message_count(const char *connection_name) const;
    void print(void);

/* ===================================
 * accessors:
 * =================================== */
public:

/* ===================================
 * status:
 * =================================== */
public:


/* ===================================
 * operators:
 * =================================== */
public:

/* ===================================
 * private methods:
 * =================================== */
protected:

/* ===================================
 * data:
 * =================================== */
protected:
    dict *m_message_dictionary;
};

}

#endif /* __MESSAGE_CACHE_H__ */
