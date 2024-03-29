/*
 * Copyright (c) 2017-2020, Wen Xiongchang <udc577 at 126 dot com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <typeinfo>
#include <string>
#include <vector>

#include "base/ca_inner_necessities.h"
#include "base/ca_return_code.h"

CA_LIB_NAMESPACE_BEGIN

class str : public no_instance
{
public:

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

    template<typename T>
    static CA_REENTRANT int to_string(const T &src, int str_capacity, char *dst)
    {
        if (str_capacity <= 0
            || nullptr == dst)
            return CA_RET(INVALID_PARAM_VALUE);

        memset(dst, 0, str_capacity);

        if (typeid(bool) == typeid(T))
            return snprintf(dst, str_capacity, "%s", src ? "true" : "false");

        const char *fmt = nullptr;

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
    static CA_REENTRANT int to_string(const T &src, std::string &dst)
    {
        dst.clear();

        char buf[CA_MAX_LEN_IN_STACK] = {0};
        int ret = to_string(src, sizeof(buf), buf);

        if (ret < 0)
            return ret;

        dst = buf;

        return dst.length();
    }

    template<typename T>
    static CA_REENTRANT std::string to_string(const T &src) // NOTE: std::to_string() is available since C++11.
    {
        std::string dst;

        to_string(src, dst);

        return dst;
    }

    template<typename T>
    static CA_REENTRANT int from_string(const char *src, T &dst)
    {
        if (nullptr == src)
            return CA_RET(INVALID_PARAM_VALUE);

        if (typeid(bool) == typeid(T))
        {
            if (0 == strcasecmp(src, "true")
                || 0 == strcmp(src, "1"))
                dst = true;
            else
                dst = false;

            return CA_RET_OK;
        }

        if (typeid(float) == typeid(T))
            dst = (T)strtof(src, nullptr);
        else if (typeid(double) == typeid(T))
            dst = (T)strtod(src, nullptr);
        else if (typeid(long double) == typeid(T))
            dst = (T)strtold(src, nullptr);
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
            dst = (T)strtoul(src, nullptr, 10);
        else if (typeid(long long int) == typeid(T))
            dst = (T)atoll(src);
        else if (typeid(unsigned long long int) == typeid(T))
            dst = (T)strtoull(src, nullptr, 10);
        else
            return CA_RET(INVALID_PARAM_TYPE);

        return CA_RET_OK;
    }

    template<typename T>
    static CA_REENTRANT int from_string(const std::string &src, T &dst)
    {
        return from_string(src.c_str(), dst);
    }

    template<typename T>
    static CA_REENTRANT T from_string(const char *str)
    {
        T dst;

        from_string(str, dst);

        return dst;
    }

    template<typename T>
    static CA_REENTRANT T from_string(const std::string &str)
    {
        // return from_string(str.c_str()); // TODO: compile error, why?!
        T dst;

        from_string(str, dst);

        return dst;
    }

    // Splits a long string @src with a length @src_len and divided by @delim
    // into short strings stored in a @result vector.
    // For example: "abc,def,gh" can be split into "abc", "def" and "gh" when @delim is ",".
    static CA_REENTRANT int split(const char *src,
        const int src_len,
        const char *delim,
        std::vector<std::string> &result) CA_NOTNULL(1, 3);

    // Same as the above, except that this one returns the result directly through the return value.
    static inline CA_REENTRANT std::vector<std::string> split(const char *src,
        const int src_len,
        const char *delim) CA_NOTNULL(1, 3)
    {
        std::vector<std::string> result;

        split(src, src_len, delim, result);

        return result;
    }

    static inline CA_REENTRANT int split(const std::string &src,
        const char *delim,
        std::vector<std::string> &result) CA_NOTNULL(2)
    {
        return split(src.c_str(), src.length(), delim, result);
    }

    static inline CA_REENTRANT std::vector<std::string> split(const std::string &src,
        const char *delim) CA_NOTNULL(2)
    {
        return split(src.c_str(), src.length(), delim);
    }

    // Gets the directory part out of the string specified by @path, and save it into @result.
    // Returns the length of result directory on success, or a negative number on error.
    // NOTE:
    //    1) If @path_len is less than the actual length of @path, then a sub-string of @path will be handled;
    //        if @path_len is greater than the actual length of @path, the result is undefined.
    //    2) Size of @result should be large enough to hold the result contents.
    static CA_REENTRANT int get_directory(const char *path, const int path_len, char *result, const char dir_delim = '/') CA_NOTNULL(1, 3);

    // Same as the above, except that this one returns the result directly through the return value.
    static inline CA_REENTRANT std::string get_directory(const char *path, const int path_len, const char dir_delim = '/') CA_NOTNULL(1)
    {
        if (path_len <= 0/* || nullptr == path*/)
            return "";

        std::string result(path_len + 1, '\0');
        char *result_ptr = (char*)result.c_str();
        int dir_len = get_directory(path, path_len, result_ptr, dir_delim);

        if (dir_len > 0 && dir_len != path_len)
            result.resize(dir_len);

        return result;
    }

    // Same as the above, except that parameter type if this one is std::string.
    static inline CA_REENTRANT std::string get_directory(const std::string &path, const char dir_delim = '/')
    {
        return get_directory(path.c_str(), path.length(), dir_delim);
    }

    // A Bash-like function, same as get_directory() with the same parameters
    static inline CA_REENTRANT std::string dirname(const char *path, const int path_len, const char dir_delim = '/') CA_NOTNULL(1)
    {
        return get_directory(path, path_len, dir_delim);
    }

    static inline CA_REENTRANT std::string dirname(const std::string &path, const char dir_delim = '/')
    {
        return get_directory(path.c_str(), path.length(), dir_delim);
    }

    static CA_REENTRANT std::pair<std::string, std::string> split_dir_and_basename(const std::string &path,
        const char dir_delim = '/',
        const char *basename_suffix = nullptr,
        const bool is_case_sensitive = true);

    // TODO: split_dir_and_basename(const char *path, ...);

    static inline CA_REENTRANT std::string basename(const std::string &path,
        const char dir_delim = '/',
        const char *basename_suffix = nullptr,
        const bool is_case_sensitive = true)
    {
        return split_dir_and_basename(path, dir_delim, basename_suffix, is_case_sensitive).second;
    }

    // TODO: basename(const char *path, ...);

    static CA_REENTRANT std::string get_absolute_path(const char *path) CA_NOTNULL(1);

    static CA_REENTRANT std::string get_self_absolute_path(void);

}; // class str

typedef str::char_less_than_operator                    char_less_than_operator;
typedef str::char_lt_op                                 char_lt_op;
typedef str::char_greater_than_operator                 char_greater_than_operator;
typedef str::char_gt_op                                 char_gt_op;
typedef str::char_equal_operator                        char_equal_operator;
typedef str::char_eq_op                                 char_eq_op;
typedef str::char_not_equal_operator                    char_not_equal_operator;
typedef str::char_ne_op                                 char_ne_op;

//using str::to_string;
//using str::from_string;
//using str::split;

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_STRING_OPERATIONS_H__ */
