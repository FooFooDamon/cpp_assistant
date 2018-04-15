/*
 * Copyright (c) 2017, Wen Xiongchang <udc577 at 126 dot com>
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
 * string.h
 *
 *  Created on: 2017-09-22
 *      Author: wenxiongchang
 * Description: An imitator std::string when CA_USE_STL is not defined,
 *              or exactly std::string when CA_USE_STL is defined.
 */

#ifndef __CPP_ASSISTANT_STL_STRING_H__
#define __CPP_ASSISTANT_STL_STRING_H__

#ifdef CA_USE_STL

#include <string>

#else

#include "allocator.h"
#include "char_traits.h"

namespace std
{

// TODO: Unfinished, just for passing the compilation
template<typename char_t, typename traits = char_traits<char_t>,
    typename alloc = allocator<char_t> >
class basic_string
{
public:
    typedef unsigned long size_type;

    static const size_type  npos = static_cast<size_type>(-1);

    basic_string();
    explicit basic_string(const alloc& a);
    basic_string(const basic_string& str);
    basic_string(const basic_string& str, size_type pos, size_type n = npos);
    basic_string(const basic_string& str, size_type pos, size_type n, const alloc& a);
    basic_string(const char_t* s, size_type n, const alloc& a = alloc());
    basic_string(const char_t* s, const alloc& a = alloc());
    basic_string(size_type n, char_t __c, const alloc& a = alloc());
#if __cplusplus >= 201103L
    basic_string(basic_string&& str);
    basic_string(initializer_list<char_t> l, const alloc& a = alloc());
    basic_string(const basic_string& str, const alloc& a);
    basic_string(basic_string&& str, const alloc& a);
#endif // #if __cplusplus >= 201103L

    basic_string& operator=(const basic_string& str);
    basic_string& operator=(const char_t* s);
    basic_string& operator=(char_t c);
#if __cplusplus >= 201103L
    basic_string& operator=(basic_string&& str);
    basic_string& operator=(initializer_list<char_t> l);
#endif // #if __cplusplus >= 201103L

    basic_string& assign(const basic_string& str);
#if __cplusplus >= 201103L
    basic_string& assign(basic_string&& str);
    basic_string& assign(initializer_list<char_t> l);
#endif // #if __cplusplus >= 201103L
    basic_string& assign(const basic_string& str, size_type pos, size_type n);
    basic_string& assign(const char_t* s, size_type n);
    basic_string& assign(const char_t* s);
    basic_string& assign(size_type n, char_t c);

    basic_string& append(const basic_string& str);
    basic_string& append(const basic_string& str, size_type pos, size_type n);
    basic_string& append(const char_t* s, size_type n);
    basic_string& append(const char_t* s);
    basic_string& append(size_type n, char_t c);
#if __cplusplus >= 201103L
    basic_string& append(initializer_list<char_t> l);
#endif // #if __cplusplus >= 201103L

    bool empty() const;

    void clear(void);
    size_type length() const;
    const char_t* c_str() const;

    basic_string& operator+=(const basic_string& str);
    basic_string& operator+=(const char * s);
    basic_string& operator+=(char_t c);
#if __cplusplus >= 201103L
    basic_string& operator+=(initializer_list<char_t> l)
#endif // #if __cplusplus >= 201103L
};

typedef basic_string<char> string;

} // namespace std

#endif // #ifdef CA_USE_STL

#endif /* __CPP_ASSISTANT_STL_STRING_H__ */
