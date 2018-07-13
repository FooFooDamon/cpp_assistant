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
 * threading.h
 *
 *  Created on: 2017/09/17
 *      Author: wenxiongchang
 * Description: Linux version of threading.h.
 */

#ifndef __CPP_ASSISTANT_PLATFORMS_LINUX_THREADING_H__
#define __CPP_ASSISTANT_PLATFORMS_LINUX_THREADING_H__

#include <sys/types.h>
#include <pthread.h>
#if CA_SINCE_CPP_11
#include <mutex>
#endif

#include "cpp_assistant/native/base/ca_inner_necessities.h"

CA_LIB_NAMESPACE_BEGIN

inline pid_t gettid(void)
{
    return pthread_self();
    //return ::gettid(); // TODO: may cause exceptions when being called within a signal handling function.
}

#if CA_SINCE_CPP_11

using std::mutex;
using std::lock_guard;

#else

class mutex : public noncopyable
{
public:
	mutex()
	{
		;
	}

	~mutex()
	{
		pthread_mutex_destroy(&m_native_mutex);
	}

	typedef pthread_mutex_t* 			native_handle_type;

	inline void lock(void)
	{
		pthread_mutex_lock(&m_native_mutex); // TODO: throw exception if failed.
	}

	inline bool try_lock(void)
	{
		return (0 == pthread_mutex_trylock(&m_native_mutex));
	}

	inline void unlock(void)
	{
		pthread_mutex_unlock(&m_native_mutex);
	}

	inline native_handle_type native_handle(void)
	{
		return &m_native_mutex;
	}

private:
	pthread_mutex_t m_native_mutex = PTHREAD_MUTEX_INITIALIZER;
};

template<typename T>
class lock_guard : public noncopyable
{
public:
	explicit lock_guard(T& target_mutex)
		: m_mutex(target_mutex)
	{
		m_mutex.lock();
	}

	// TODO: lock_guard(T& target_mutext, adopt_lock_t)

	~lock_guard()
	{
		m_mutex.unlock();
	}

private:
	lock_guard();

private:
	T& m_mutex;
};

#endif // if CA_SINCE_CPP_11

CA_LIB_NAMESPACE_END

#endif /* __CPP_ASSISTANT_PLATFORMS_LINUX_THREADING_H__ */
