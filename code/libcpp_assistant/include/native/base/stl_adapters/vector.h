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
 * vector.h
 *
 *  Created on: 2017-09-23
 *      Author: wenxiongchang
 * Description: An imitator of std::vector when CA_USE_STL is not defined,
 *              or exactly std::vector when CA_USE_STL is defined.
 */

#ifndef __CPP_ASSISTANT_STL_VECTOR_H__
#define __CPP_ASSISTANT_STL_VECTOR_H__

#ifdef CA_USE_STL

#include <vector>

#else

#include <stddef.h>

#include "allocator.h"

namespace std
{

// TODO: Unfinished, just for passing the compilation
template<typename type, typename alloc = std::allocator<type> >
class vector
{
public:
    typedef type value_type;
    typedef size_t size_type;

    void push_back(const value_type& x);

    void clear(void);

    size_type size(void) const;

    bool empty(void) const;

    type& operator[](size_type n);
    const type& operator[](size_type n) const;
};

} // namespace std

#endif // #ifdef CA_USE_STL

#endif /* __CPP_ASSISTANT_STL_VECTOR_H__ */
