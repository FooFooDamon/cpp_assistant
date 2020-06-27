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

#include "ca_string.h"

#include <stdlib.h>

#include "private/debug.h"
#include "base/platforms/os_specific.h"

CA_LIB_NAMESPACE_BEGIN

/*static */CA_REENTRANT int str::split(const char *src,
    const int src_len,
    const char *delim,
    std::vector<std::string> &result)/* CA_NOTNULL(1, 3) */
{
    result.clear();

    if (src_len <= 0
        || nullptr == src
        || nullptr == delim)
        return CA_RET(INVALID_PARAM_VALUE);

    bool is_long_str = (src_len + 1 > CA_MAX_LEN_IN_STACK);
    char *buf_heap = nullptr;

    if (is_long_str)
    {
        buf_heap = (char*)calloc(src_len + 1, sizeof(char));
        if (nullptr == buf_heap)
            return CA_RET(MEMORY_ALLOC_FAILED);
    }

    char buf_stack[CA_MAX_LEN_IN_STACK] = {0};
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

/*static */CA_REENTRANT int str::get_directory(const char *path, const int path_len, char *result, const char dir_delim/* = '/'*/)/* CA_NOTNULL(1, 3) */
{
    if (path_len <= 0
        || nullptr == path
        || nullptr == result)
        return CA_RET(INVALID_PARAM_VALUE);

    int pos = path_len;

    while (--pos >= 0 &&  path[pos] != dir_delim);

    if (pos < 0)
    {
        if (0 == strcmp(path, ".."))
        {
            strncpy(result, "..", 3);

            return 2;
        }

        strncpy(result, ".", 2);

        return 1;
    }

    size_t dir_len = pos;

    if (0 == dir_len)
    {
        strncpy(result, "/", 2);

        return 1;
    }

    strncpy(result, path, dir_len);
    result[dir_len] = '\0';

    return dir_len;
}

/*static */CA_REENTRANT std::pair<std::string, std::string> str::split_dir_and_basename(const std::string &path,
    const char dir_delim/* = '/'*/,
    const char *basename_suffix/* = nullptr*/,
    const bool is_case_sensitive/* = true*/)
{
    const char *path_ptr = path.c_str();
    int path_len = path.length();
    const std::string &dir = get_directory(path_ptr, path_len, dir_delim);
    size_t dir_len = dir.length();

    if (0 == dir_len)
        return std::make_pair("", "");

    bool path_is_pure_dir = (dir_delim == path[path_len - 1]) || ("." == path) || (".." == path);

    if (path_is_pure_dir)
        return std::make_pair(dir, "");

    int basename_start_pos = (dir_delim == path[dir_len]) ? (dir_len + 1) : 0;
    const std::string &basename = path.substr(basename_start_pos);

    if (nullptr == basename_suffix)
        return std::make_pair(dir, basename);

    size_t basename_len = basename.length();
    size_t last_dot_pos = basename.rfind(".");

    if (0 == last_dot_pos || std::string::npos == last_dot_pos)
        return std::make_pair(dir, basename);

    if ((basename_len - 1 == last_dot_pos && 0 == strcmp(".", basename_suffix))
        || 0 == strcmp(".*", basename_suffix)
        || (is_case_sensitive && 0 == basename.substr(last_dot_pos + 1).compare(basename_suffix + 1))
        || (!is_case_sensitive && 0 == strcasecmp(basename.substr(last_dot_pos + 1).c_str(), basename_suffix + 1)))
        return std::make_pair(dir, basename.substr(0, last_dot_pos));

    return std::make_pair(dir, basename);
}

/*static */CA_REENTRANT std::string str::get_absolute_path(const char *path)/* CA_NOTNULL(1) */
{
    if (nullptr == path || strlen(path) > CA_MAX_PATH_LEN)
        return "";

    char result[CA_MAX_PATH_LEN + 1] = {0};

    if (nullptr == realpath(path, result))
        return "";

    return result;
}

/*static */CA_REENTRANT std::string str::get_self_absolute_path(void)
{
    char result[CA_MAX_PATH_LEN + 1] = {0};

    if (readlink("/proc/self/exe", result, CA_MAX_PATH_LEN) < 0)
        return "";

    return result;
}

CA_LIB_NAMESPACE_END
