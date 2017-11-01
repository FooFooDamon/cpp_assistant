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

#include "file_logger.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>

#include <typeinfo>

#include "private/debug.h"
#include "base/platforms/os_specific.h"

CA_LIB_NAMESPACE_BEGIN

DEFINE_CLASS_NAME(FileLogger);
static const int S_INITIAL_FILE_LOG_NUM = 0;

FileLogger::FileLogger()
    : m_name_len_limit(LOG_NAME_LEN_LIMIT_DEFAULT)
    , m_dir_len_limit(LOG_DIR_LEN_LIMIT_DEFAULT)
    , m_line_limit(LOG_LINE_LIMIT_DEFAULT)
    , m_log_name(NULL)
    , m_log_directory(NULL)
    , m_buffer(NULL)
    //, m_to_screen(false) // error??
{
    m_to_screen = false;
}

FileLogger::~FileLogger()
{
    Close();

    if (NULL != m_log_name)
    {
        free(m_log_name);
        m_log_name = NULL;
    }

    if (NULL != m_log_directory)
    {
        free(m_log_directory);
        m_log_directory = NULL;
    }
}

/*virtual */int FileLogger::Open(int cache_buf_size/* = DEFAULT_LOG_CACHE_SIZE*/)/*  = 0 */
{
    if (is_open())
        return CA_RET_OK;

    if (NULL == m_log_name || NULL == m_log_directory)
    {
        cerror("m_log_name or m_log_directory not set\n");
        return CA_RET(TARGET_NOT_READY);
    }

    if ('\0' == m_log_name[0] || '\0' == m_log_directory[0])
        return CA_RET(INVALID_PATH);

    int err = 0;
    int path_len = directory_length_limit() + name_length_limit() + 1;
    char *file = (char *)calloc(path_len, sizeof(char));

    snprintf(file, path_len, "%s%c%s.tmp", m_log_directory, get_directory_delimiter(), m_log_name);
    cdebug("opening %s ...\n", file);
    m_output_holder = fopen(file, "w");
    free(file);
    file = NULL;
    if (NULL == m_output_holder)
    {
        cerror("fopen() failed\n");
        err = errno;
        return (0 != err) ? (-err) : CA_RET_GENERAL_FAILURE;
    }
    cdebug("done.\n");

    /*
     * Sets cache buffer so that
     * log contents are flushed in batches.
     * NOTE: If the buf parameter of setvbuf() is null,
     *   then the bufsize specified by user may be ineffective
     *   or no larger than buffer size determined by system.
     *   To do a large size buffering, allocate your own buffer
     *   and assign it to setvbuf().
     */
#if 1
    if (NULL == m_buffer && NULL == (m_buffer = (char *)malloc(cache_buf_size)))
    {
        cerror("malloc() for cache buffer failed\n");
        fclose(m_output_holder);
        return CA_RET(MEMORY_ALLOC_FAILED);
    }
    cdebug("buffer creation ok\n");

    if (0 != setvbuf(m_output_holder, m_buffer, _IOFBF, cache_buf_size))
#else
    if (0 != setvbuf(m_output_holder, NULL, _IOFBF, cache_buf_size))
#endif
    {
        err = errno;
        cerror("setvbuf() failed\n");
        fclose(m_output_holder);
        free(m_buffer);
        m_buffer = NULL;
        return (0 != err) ? (-err) : CA_RET_GENERAL_FAILURE;
    }

    m_is_open = true;
    m_cur_line = 0;

    return CA_RET_OK;
}

/*virtual */int FileLogger::Close(bool release_buffer/* = true*/)/*  = 0 */
{
    if (!is_open())
        return CA_RET(OK);

    fflush(m_output_holder);
    fclose(m_output_holder);
    m_output_holder = NULL;

    if (m_output_holder == __get_debug_output_holder())
        __set_debug_output(stdout);

    if (m_output_holder == __get_error_output_holder())
        __set_error_output(stderr);

    if (release_buffer && NULL != m_buffer)
    {
        free(m_buffer);
        m_buffer = NULL;
    }

    m_is_open = false;

    int path_len = directory_length_limit() + name_length_limit() + 1;
    char *src_file = (char *)calloc(path_len, sizeof(char));
    char *dst_file = (char *)calloc(path_len, sizeof(char));
    const char DIR_DELIM = get_directory_delimiter();

    snprintf(src_file, path_len, "%s%c%s.tmp", m_log_directory, DIR_DELIM, m_log_name);
    snprintf(dst_file, path_len, "%s%c%s", m_log_directory, DIR_DELIM, m_log_name);

    __UpdateLogNum(); // Updates some info for the next opening in advance.

    if (rename(src_file, dst_file) < 0)
    {
        int err = errno;

        cerror("rename(%s -> %s) failed\n", src_file, dst_file);
        free(src_file);
        free(dst_file);

        return (0 != err) ? (-err) : CA_RET_GENERAL_FAILURE;
    }
    cdebug("rename(%s -> %s) successful\n", src_file, dst_file);
    free(src_file);
    free(dst_file);

    return CA_RET_OK;
}

/*virtual */int FileLogger::set_log_directory(const char *dir) /*  CA_NOTNULL(2) */
{
    if (NULL == dir)
        return CA_RET(NULL_PARAM);

    if (is_open())
        return CA_RET(DEVICE_BUSY);

    int len = strlen(dir);
    int dir_len_limit = directory_length_limit();

    if (len > dir_len_limit)
        return CA_RET(PATH_TOO_LONG);

    int perm = R_OK | W_OK;

    if (access(dir, perm) < 0)
    {
        int err = errno;

        if (ENOENT == err)
        {
            cdebug("directory[%s] does not exist, try to create one.\n", dir);
            mkdir(dir, 0777); // It's not required, but do it for upper-layer convenience.
        }

        if (access(dir, perm) < 0) // Checks it again.
        {
            err = errno;
            cerror("failed to access %s with permission 0x%03X\n", dir, perm);

            return (0 != err) ? (-err) : CA_RET_GENERAL_FAILURE;
        }
    }

    /*
     * Updates m_log_directory: MUST be after the checking above in order to get an available directory!
     */

    int max_dir_len = dir_len_limit + 1;

    if (NULL == m_log_directory)
        m_log_directory = (char*)malloc(max_dir_len);

    if (NULL == m_log_directory)
    {
        cerror("malloc() for m_log_directory failed\n");
        return CA_RET(MEMORY_ALLOC_FAILED);
    }

    //memset(m_log_directory, 0, max_dir_len);
    strncpy(m_log_directory, dir, len);
    if (get_directory_delimiter() == m_log_directory[len - 1])
        m_log_directory[len - 1] = '\0'; // removes the trailing delimiter '/' or '\'

    return CA_RET_OK;
}

/*virtual */int FileLogger::set_log_name(const char *base_name)/*  CA_NOTNULL(2)  = 0 */
{
    if (NULL == base_name)
        return CA_RET(NULL_PARAM);

    if (is_open())
        return CA_RET(DEVICE_BUSY);

    const int RESERVED_LEN = 64; // For other parts of a log name, such as date info, PID, etc.
    int basename_len = strlen(base_name);
    int name_len_limit = name_length_limit();

    if (basename_len > name_len_limit - RESERVED_LEN)
        return CA_RET(PATH_TOO_LONG);

    int max_name_len = name_len_limit + 1;

    if (NULL == m_log_name)
        m_log_name = (char*)malloc(max_name_len);

    if (NULL == m_log_name)
    {
        cerror("malloc() for m_log_name failed\n");
        return CA_RET(MEMORY_ALLOC_FAILED);
    }

    bool is_same_name = ( ('_' == m_log_name[basename_len])
        && (0 == strncmp(m_log_name, base_name, basename_len)) );

    if (is_same_name)
    {
        cdebug("base name unchanged, no need to set, current full name: %s,"
            " input base name: %s\n", m_log_name, base_name);
        return CA_RET_OK;
    }

    m_log_num = S_INITIAL_FILE_LOG_NUM; // Log number has to be set to initial value for a new name!

    __InnerlySetLogName(base_name);

    return CA_RET_OK;
}

/*virtual */int FileLogger::__SwitchLoggerStatus(void)
{
    int ret = CA_RET_OK;
    bool is_used_by_inner_debug = (m_output_holder == __get_debug_output_holder());
    bool is_used_by_inner_error = (m_output_holder == __get_error_output_holder());

    if (CA_RET_OK != (ret = Close(NOT_RELEASE_LOG_BUF_ON_CLOSE))) // Closes it first.
        return ret;

    if (CA_RET_OK != (ret = Open())) // Re-opens the logger to start a new log file.
        return ret;

    if (is_used_by_inner_debug)
        __set_debug_output(m_output_holder); // Recovers inner debug.

    if (is_used_by_inner_error)
        __set_error_output(m_output_holder); // Recovers inner error report.

    m_cur_line = 0;

    return ret;
}

// NOTE: The main purpose of this function is to update log number,
//     and the whole log name is also updated relatively.
int FileLogger::__UpdateLogNum(void)
{
    int max_name_len = name_length_limit() + 1;
    char *base_name = (char *)calloc(max_name_len, sizeof(char));
    char date[16] = {0};
    char *date_ptr = NULL;

    snprintf(date, sizeof(date), "_%04d%02d%02d",
        m_date.tm_year + 1900, m_date.tm_mon + 1, m_date.tm_mday);
    date_ptr = strstr(m_log_name, date);

    if (NULL == date_ptr)
        strncpy(base_name, m_log_name, max_name_len);
    else
        strncpy(base_name, m_log_name, (size_t)(date_ptr - m_log_name));

    __InnerlySetLogName(base_name, true); // Refreshes the whole name.

    return CA_RET_OK;
}

int FileLogger::__InnerlySetLogName(const char *base_name, bool updates_log_num/* = false*/)
{
    pid_t pid = getpid();
    struct timeval tv;
    struct tm now;
    int max_name_len = name_length_limit() + 1;

    gettimeofday(&tv, NULL);
    localtime_r((time_t *)&(tv.tv_sec), &now);
    if (updates_log_num)
    {
        if (now.tm_mday != m_date.tm_mday)
        {
            m_log_num = S_INITIAL_FILE_LOG_NUM;
            cdebug("tm_mday switched, set m_log_num = %d\n", m_log_num);
        }
        else
        {
            ++m_log_num;
            cdebug("tm_mday not switched, ++m_log_num and set m_log_num = %d\n", m_log_num);
        }
    }
    memcpy(&m_date, &now, sizeof(struct tm));

    //memset(m_log_name, 0, max_name_len);
    snprintf(m_log_name, max_name_len, "%s_%04d%02d%02d_%d_%d.log",
        base_name, m_date.tm_year + 1900, m_date.tm_mon + 1, m_date.tm_mday, pid, m_log_num);
    cdebug("m_log_name is %s\n", m_log_name);

    return CA_RET_OK;
}

int FileLogger::__AdjustPathLengthLimit(const int max_limit,
    const int min_limit,
    const int target_limit,
    int &path_limit_var,
    char* &path_holder_var)
{
    if (is_open())
        return CA_RET(DEVICE_BUSY);

    int limit;
    char *tmp = NULL;

    if (target_limit < min_limit)
        limit = min_limit;
    else if (target_limit > max_limit)
        limit = max_limit;
    else
        limit = target_limit;

    if (NULL == path_holder_var)
        goto ADJUST_ACTION;

    if (limit == path_limit_var)
        return CA_RET_OK;

    if (limit < path_limit_var)
    {
        path_holder_var[limit] = '\0'; // truncates the current path string.
        goto ADJUST_ACTION;
    }

    /*
     * when limit > path_limit_var:
     */

    if (NULL == (tmp = (char *)calloc(limit + 1, sizeof(char))))
        return CA_RET(MEMORY_ALLOC_FAILED);

    memcpy(tmp, path_holder_var, path_limit_var + 1);
    free(path_holder_var);
    path_holder_var = tmp;

ADJUST_ACTION:

    path_limit_var = limit;

    return CA_RET_OK;
}

CA_LIB_NAMESPACE_END
