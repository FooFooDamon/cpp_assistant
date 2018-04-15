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
 * map.h
 *
 *  Created on: 2017-09-23
 *      Author: wenxiongchang
 * Description: Imitators of std::map and std::multimap when CA_USE_STL is not defined,
 *              or exactly std::map and std::multimap when CA_USE_STL is defined.
 */

#ifndef __CPP_ASSISTANT_STL_MAP_H__
#define __CPP_ASSISTANT_STL_MAP_H__

#ifdef CA_USE_STL

#include <map>

#else

#include "allocator.h"
#include "functors.h"
#include "pair.h"

namespace std
{

// TODO: Unfinished, just for passing the compilation
template<typename key, typename type, typename compare = std::less<key>,
    typename alloc = std::allocator<std::pair<const key, type> > >
class map
{
public:
    typedef key key_type;
    typedef type mapped_type;
    typedef std::pair<const key, type> value_type;
    typedef compare key_compare;
    typedef alloc allocator_type;
    typedef unsigned long size_type;
    typedef value_type* iterator; // not strictly correct
    typedef const value_type* const_iterator;

    iterator find(const key_type& x);
    const_iterator find(const key_type& x) const;

    void clear(void);

    size_type erase(const key_type& x);

    iterator begin();
    const_iterator begin() const;

    iterator end();
    const_iterator end() const;

    std::pair<iterator, bool> insert(const value_type& x);
    std::pair<iterator, bool> insert(const std::pair<key, type>& x);

    mapped_type& operator[](const key_type& k);
};

} // namespace std

#endif // #ifdef CA_USE_STL

#endif /* __CPP_ASSISTANT_STL_MAP_H__ */
