/*
 * Copyright (c) 2017-2018, Wen Xiongchang <udc577 at 126 dot com>
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
 * ca_string.h
 *
 *  Created on: 2017/09/22
 *      Author: wenxiongchang
 * Description: APIs for string operations.
 */

#ifndef __CPP_ASSISTANT_STRING_OPERATIONS_H__
#define __CPP_ASSISTANT_STRING_OPERATIONS_H__

#include <stdlib.h>
#include <string.h>

#include <typeinfo>
#include <string>
#include <vector>

#include "base/ca_inner_necessities.h"
#include "base/ca_return_code.h"

CA_LIB_NAMESPACE_BEGIN

typedef struct char_less_than_operator
{
    bool operator()(const char *s1, const char *s2) const
    {
        return strcmp(s1, s2) < 0;
    }
}char_less_than_operator;

typedef char_less_than_operator char_lt_op;

typedef struct char_greater_than_operator
{
    bool operator()(const char *s1, const char *s2) const
    {
        return strcmp(s1, s2) > 0;
    }
}char_greater_than_operator;

typedef char_greater_than_operator char_gt_op;

typedef struct char_equal_operator
{
    bool operator()(const char *s1, const char *s2) const
    {
        return strcmp(s1, s2) == 0;
    }
}char_equal_operator;

typedef char_equal_operator char_eq_op;

typedef struct char_not_equal_operator
{
    bool operator()(const char *s1, const char *s2) const
    {
        return strcmp(s1, s2) != 0;
    }
}char_not_equal_operator;

typedef char_not_equal_operator char_ne_op;

namespace str
{

enum
{
    MAX_STRING_LEN_IN_STACK = 4 * 1024
};

template<typename T>
CA_REENTRANT int to_string(const T &src, int str_capacity, char *dst)
{
    if (str_capacity <= 0
        || NULL == dst)
        return CA_RET(INVALID_PARAM_VALUE);

    const char *fmt = NULL;

    if (typeid(float) == typeid(T))
        fmt = "%f";
    else if (typeid(double) == typeid(T))
        fmt = "%lf";
    else if (typeid(long double) == typeid(T))
        fmt = "%Lf";
    else if (typeid(char) == typeid(T))
        fmt = "%hhd";
    else if (typeid(unsigned char) == typeid(T))
        fmt = "%hhu";
    else if (typeid(short int) == typeid(T))
        fmt = "%hd";
    else if (typeid(unsigned short int) == typeid(T))
        fmt = "%hu";
    else if (typeid(int) == typeid(T))
        fmt = "%d";
    else if (typeid(unsigned int) == typeid(T))
        fmt = "%u";
    else if (typeid(long int) == typeid(T))
        fmt = "%ld";
    else if (typeid(unsigned long int) == typeid(T))
        fmt = "%lu";
    else if (typeid(long long int) == typeid(T))
        fmt = "%lld";
    else if (typeid(unsigned long long int) == typeid(T))
        fmt = "%llu";
    else
        return CA_RET(INVALID_PARAM_TYPE);

    return snprintf(dst, str_capacity, fmt, src);
}

template<typename T>
CA_REENTRANT int to_string(const T &src, std::string &dst)
{
    dst.clear();

    char buf[MAX_STRING_LEN_IN_STACK] = {0};
    int ret = to_string(src, sizeof(buf), buf);

    if (ret < 0)
        return ret;

    dst = buf;

    return dst.length();
}

template<typename T>
CA_REENTRANT std::string to_string(const T &src)
{
    std::string dst;

    to_string(src, dst);

    return dst;
}

template<typename T>
CA_REENTRANT int from_string(const char *src, T &dst)
{
    if (NULL == src)
        return CA_RET(INVALID_PARAM_VALUE);

    if (typeid(float) == typeid(T))
        dst = (T)strtof(src, NULL);
    else if (typeid(double) == typeid(T))
        dst = (T)strtod(src, NULL);
    else if (typeid(long double) == typeid(T))
        dst = (T)strtold(src, NULL);
    else if (typeid(char) == typeid(T))
        dst = (T)atoi(src); // TODO: correct?
    else if (typeid(unsigned char) == typeid(T))
        dst = (T)atoi(src); // TODO: correct?
    else if (typeid(short int) == typeid(T))
        dst = (T)atoi(src); // TODO: correct?
    else if (typeid(unsigned short int) == typeid(T))
        dst = (T)atoi(src); // TODO: correct?
    else if (typeid(int) == typeid(T))
        dst = (T)atoi(src);
    else if (typeid(unsigned int) == typeid(T))
        dst = (T)atoi(src); // TODO: correct?
    else if (typeid(long int) == typeid(T))
        dst = (T)atol(src);
    else if (typeid(unsigned long int) == typeid(T))
        dst = (T)strtoul(src, NULL, 10);
    else if (typeid(long long int) == typeid(T))
        dst = (T)atoll(src);
    else if (typeid(unsigned long long int) == typeid(T))
        dst = (T)strtoull(src, NULL, 10);
    else
        return CA_RET(INVALID_PARAM_TYPE);

    return CA_RET_OK;
}

template<typename T>
CA_REENTRANT int from_string(const std::string &src, T &dst)
{
    return from_string(src.c_str(), dst);
}

template<typename T>
CA_REENTRANT T from_string(const char *str)
{
    T dst;

    from_string(str, dst);

    return dst;
}

template<typename T>
CA_REENTRANT T from_string(const std::string &str)
{
    // return from_string(str.c_str()); // TODO: compile error, why?!
    T dst;

    from_string(str, dst);

    return dst;
}

// Splits a long string @src with a length @src_len and divided by @delim
// into short strings stored in a @result vector.
// For example: "abc,def,gh" can be split into "abc", "def" and "gh" when @delim is ",".
CA_REENTRANT int split(const char *src,
    const int src_len,
    const char *delim,
    std::vector<std::string> &result);

} // namespace str

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_STRING_OPERATIONS_H__ */
