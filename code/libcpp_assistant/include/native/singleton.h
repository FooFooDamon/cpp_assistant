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
 * singleton.h
 *
 *  Created on: 2017-09-22
 *      Author: wenxiongchang
 * Description: One of implementations, which uses template technology, for singleton .
 */

#ifndef __CPP_ASSISTANT_SINGLETON_H__
#define __CPP_ASSISTANT_SINGLETON_H__

#include "base/ca_inner_necessities.h"
#include "base/platforms/threading.h"

CA_LIB_NAMESPACE_BEGIN

/*
 * singleton<T>: A thread-safe class template.
 * Usage examples:
 *     (1):
 *         class YourClass : public singleton<YourClass>
 *         {
 *         private: // or protected
 *             YourClass(){ // Your implementation }
 *             YourClass(const YourClass& src);
 *             YourClass& operator=(const YourClass& src);
 *             ~YourClass(){ // Your implementation }
 *             friend singleton<YourClass>;
 *
 *         // Others.
 *         };
 *         YourClass *instance = YourClass::get_instance();
 *
 *     (2):
 *         YourClass *instance = singleton<YourClass>::get_instance();
 */

template<typename T>
class singleton
{
/* ===================================
 * constructors:
 * =================================== */
//private:
protected: // for inheritance
    singleton(){}

/* ===================================
 * copy control:
 * =================================== */
protected:
    singleton(const singleton& src);
    singleton& operator=(const singleton& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    // No destructor needed for singleton itself, see implementation of get_instance().
    // TODO: Or needs a empty virtual destructor ??

/* ===================================
 * types:
 * =================================== */
public:
    // place types, enumerations for example, here

/* ===================================
 * abilities:
 * =================================== */
public:
    static CA_THREAD_SAFE inline T* get_instance(void)
    {
        if (NULL == m_instance)
        {
            lock_guard<mutex> lock(m_lock);
            if (NULL == m_instance)
            {
                static T ins; // Constructor of a common class or a derived class has to be accessible to singleton<T>.
                m_instance = &ins;
            }
        }

        return m_instance;
    }

/* ===================================
 * attributes:
 * =================================== */
public:

/* ===================================
 * status:
 * =================================== */
public:
    // place functions changing or showing status of the class here


/* ===================================
 * operators:
 * =================================== */
public:
    // place any overloaded operators here

/* ===================================
 * private methods:
 * =================================== */
protected:
    // place functions used by this class only here

/* ===================================
 * data:
 * =================================== */
private:
    static T* m_instance;
    static mutex m_lock;
};

template<typename T> T* singleton<T>::m_instance = NULL;
template<typename T> mutex singleton<T>::m_lock;

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_SINGLETON_H__ */
