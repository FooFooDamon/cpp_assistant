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
 * system.h
 *
 *  Created on: 2017-09-22
 *      Author: wenxiongchang
 * Description: System relative definitions and APIs.
 */

#ifndef __CPP_ASSISTANT_SYSTEM_H__
#define __CPP_ASSISTANT_SYSTEM_H__

#include <stdint.h>
#include <string.h>

#include "base/ca_inner_necessities.h"
#include "base/ca_return_code.h"

CA_LIB_NAMESPACE_BEGIN

class sys : public no_instance
{
public:
    static inline CA_REENTRANT bool host_is_big_endian(void)
    {
        const int16_t TEST_NUM = 0x0102;
        static char s_test_buf[2] = {0};

        if (0 == s_test_buf[0])
            memcpy(s_test_buf, (char*)&TEST_NUM, 2);

        return (0x01 == s_test_buf[0]);
    }

#if defined(WINDOWS)

    // Converts the unsigned short integer @host16 from host byte order to network byte order.
    static inline uint16_t htons(uint16_t host16)
    {
        return byte_order_conversion(host16);
    }

    // Converts the unsigned short integer @net16 from network byte order to host byte order.
    static inline uint16_t ntohs(uint16_t net16)
    {
        return byte_order_conversion(net16);
    }

    // Converts the unsigned integer @host32 from host byte order to network byte order.
    static inline uint32_t htonl(uint32_t host32)
    {
        return byte_order_conversion(host32);
    }

    // Converts the unsigned integer @net32 from network byte order to host byte order.
    static inline uint32_t ntohl(uint32_t net32)
    {
        return byte_order_conversion(net32);
    }

#endif // if defined(WINDOWS)

    // Converts the unsigned 64-bit integer @host64 from host byte order to network byte order.
    static inline uint64_t htonl64(uint64_t host64)
    {
        return byte_order_conversion(host64);
    }

    // Converts the unsigned 64-bit integer @net64 from network byte order to host byte order.
    static inline uint64_t ntohl64(uint64_t net64)
    {
        return byte_order_conversion(net64);
    }

private:
    static inline int turn_around_memory(const void* src_data, const int src_len, void* dst_data, int& dst_len)
    {
        // NOTE: This function should not be used directly by external modules,
        //    thus needs no strict checks.
        /*if ((nullptr == src_data) || (nullptr == dst_data))
            return CA_RET(NULL_PARAM);

        if ((src_data == dst_data) || (src_len < 0) || (dst_len < src_len))
            return CA_RET(INVALID_PARAM_VALUE);*/

        dst_len = src_len;
        for (int i = 0; i < src_len; ++i)
        {
            ((char*)dst_data)[i] = ((char*)src_data)[src_len - i - 1];
        }

        return CA_RET_OK;
    }

    template< typename Integer >
    static Integer byte_order_conversion(Integer src)
    {
        if (host_is_big_endian())
            return src;

        Integer dst;
        int dst_len = sizeof(dst);

        turn_around_memory(&src, sizeof(src), &dst, dst_len);

        return dst;
    }

}; // class sys

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_SYSTEM_H__

