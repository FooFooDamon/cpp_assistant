/*
 * Copyright (c) 2017-2019, Wen Xiongchang <udc577 at 126 dot com>
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
 * noncopyable.h
 *
 *  Created on: 2018-07-07
 *      Author: wenxiongchang
 * Description: Base class of classes which can not be copied.
 */

#ifndef __CPP_ASSISTANT_NONCOPYABLE_H__
#define __CPP_ASSISTANT_NONCOPYABLE_H__

#include "basic_info.h"

CA_LIB_NAMESPACE_BEGIN

class noncopyable
{
protected:
    noncopyable() {}
    ~noncopyable() {}

private:
    noncopyable(const noncopyable &src);
    noncopyable& operator=(const noncopyable &src);
};

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_NONCOPYABLE_H__ */
