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

#include "ca_string.h"

#include <stdlib.h>

#include "private/debug.h"

CA_LIB_NAMESPACE_BEGIN

CA_REENTRANT /* static */int str::split(const char *src,
    const int src_len,
    const char *delim,
    std::vector<std::string> &result)
{
    result.clear();

    if (nullptr == src ||
        src_len <= 0 ||
        nullptr == delim)
        return CA_RET(INVALID_PARAM_VALUE);

    bool is_long_str = (src_len + 1 > MAX_LEN_IN_STACK);
    char *buf_heap = nullptr;

    if (is_long_str)
    {
        buf_heap = (char*)calloc(src_len + 1, sizeof(char));
        if (nullptr == buf_heap)
            return CA_RET(MEMORY_ALLOC_FAILED);
    }

    char buf_stack[MAX_LEN_IN_STACK] = {0};
    char *buf = is_long_str ? buf_heap : buf_stack;
    char *part = nullptr;
    char *save_ptr = nullptr;
    std::string tmp;

    //nsdebug(str, "address: buf_stack[%p], buf_heap[%p], buf[%p]\n", buf_stack, buf_heap, buf);
    strncpy(buf, src, strlen(src));
    //nsdebug(str, "before splitting, buf: %s\n", buf);

    part = strtok_r(buf, delim, &save_ptr);
    while (nullptr != part)
    {
        //nsdebug(str, "part: %s\n", part);
        tmp.clear();
        tmp += part;
        try
        {
            result.push_back(tmp);
        }
        catch (std::exception& e)
        {
            nserror(str, "result.push_back() failed: %s\n", e.what());
            if (is_long_str) free(buf_heap);
            result.clear();
            return CA_RET_GENERAL_FAILURE;
        }
        part = strtok_r(nullptr, delim, &save_ptr);
    }

    //nsdebug(str, "after splitting, buf: %s\n", buf);

    if (is_long_str) free(buf_heap);

    return CA_RET_OK;
}

CA_LIB_NAMESPACE_END
