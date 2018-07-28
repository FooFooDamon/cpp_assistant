/*
 * Copyright (c) 2018, Wen Xiongchang <udc577 at 126 dot com>
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

#include "sequential_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/ca_return_code.h"
#include "private/debug.h"

CA_LIB_NAMESPACE_BEGIN

const void* sequential_buffer::OVERFLOW_PTR = nullptr;

sequential_buffer::sequential_buffer()
{
    init();
}

sequential_buffer::sequential_buffer(const sequential_buffer& src)
{
    init();
    copy_from(src);
}

sequential_buffer::sequential_buffer(int size)
{
    init();
    innerly_create(size);
}

sequential_buffer& sequential_buffer::operator=(const sequential_buffer& src)
{
    if (this != &src)
        copy_from(src);

    return *this;
}

sequential_buffer::~sequential_buffer()
{
    destroy();
}

int sequential_buffer::create(int size)
{
    if (nullptr != m_data)
        return CA_RET(OBJECT_ALREADY_EXISTS);

    return innerly_create(size);
}

void sequential_buffer::destroy(void)
{
    clear();
}

int sequential_buffer::resize(int new_size)
{
    if (new_size <= 0)
        return CA_RET(INVALID_PARAM_VALUE);

    if (new_size < MIN_BUF_SIZE)
        new_size = MIN_BUF_SIZE;

    if (m_total_size == new_size)
        return CA_RET_OK;

    void *new_buf = calloc(new_size, sizeof(char));

    if (nullptr == new_buf)
        return CA_RET(MEMORY_ALLOC_FAILED);

    int data_size = (this->data_size() <= new_size) ? this->data_size() : new_size;
    int read_pos = (OVERFLOW_POS != m_read_pos) ? m_read_pos : 0;

    if (data_size > 0)
        memcpy(new_buf, ((char *)m_data) + read_pos, data_size);

    free(m_data);
    m_data = new_buf;
    m_total_size = new_size;
    m_read_pos = 0;
    m_write_pos = data_size;

    return new_size;
}

int sequential_buffer::read(const int len, void *data)
{
    if (empty() || 0 == len)
        return 0;

    if (len < 0 || nullptr == data)
        return CA_RET(INVALID_PARAM_VALUE);

    int available_len = this->data_size();
    int read_len = (len <= available_len) ? len : available_len;
    int pos_limit = this->total_size();

    if (m_read_pos + read_len >= pos_limit)
        move_data_to_header();

    memcpy(data, ((char *)m_data) + m_read_pos, read_len);
    m_read_pos += read_len;
    if (m_read_pos >= pos_limit)
        m_read_pos = OVERFLOW_POS;

    return read_len;
}

int sequential_buffer::write(const int len, const void *data)
{
    if (full() || 0 == len)
        return 0;

    if (len < 0 || nullptr == data)
        return CA_RET(INVALID_PARAM_VALUE);

    int available_len = this->free_size();
    int write_len = (len <= available_len) ? len : available_len;
    int pos_limit = this->total_size();

    if (m_write_pos + write_len >= pos_limit)
        move_data_to_header();

    memcpy(((char *)m_data) + m_write_pos, data, write_len);
    m_write_pos += write_len;
    if (m_write_pos >= pos_limit)
        m_write_pos = OVERFLOW_POS;

    return write_len;
}

void* sequential_buffer::get_read_pointer(void)
{
    if (nullptr == m_data)
        return nullptr;

    if (has_overflown_pointer())
        move_data_to_header();

    if (OVERFLOW_POS == m_read_pos)
        return const_cast<void*>(OVERFLOW_PTR);

    return (void *)((char *)m_data + m_read_pos);
}

int sequential_buffer::move_read_pointer(const int offset)
{
    if (offset <= 0 ||
        nullptr == m_data)
        return 0;

    if (has_overflown_pointer())
        move_data_to_header();

    int data_len = data_size();

    if (data_len <= 0)
    {
        m_read_pos = 0;
        m_write_pos = 0;

        return 0;
    }

    int distance = (offset <= data_len) ? offset : data_len;

    m_read_pos += distance;
    if (m_read_pos >= total_size() || m_read_pos >= m_write_pos)
    {
        m_read_pos = 0;
        m_write_pos = 0;
    }

    return distance;
}

void* sequential_buffer::get_write_pointer(void)
{
    if (nullptr == m_data)
        return nullptr;

    if (has_overflown_pointer())
        move_data_to_header();

    if (OVERFLOW_POS == m_write_pos)
        return const_cast<void*>(OVERFLOW_PTR);

    return (void *)(((char *)m_data) + m_write_pos);
}

int sequential_buffer::move_write_pointer(const int offset)
{
    if (offset <= 0 ||
        nullptr == m_data)
        return 0;

    if (has_overflown_pointer())
        move_data_to_header();

    int free_len = total_size() - m_write_pos; // NOTE: DO NOT use free_size() directly here!

    if (free_len <= 0)
        return 0;

    int distance = (offset <= free_len) ? offset : free_len;

    m_write_pos += distance;
    if (m_write_pos >= total_size())
        m_write_pos = OVERFLOW_POS;

    return distance;
}

int sequential_buffer::move_data_to_header(void)
{
    if (OVERFLOW_POS == m_read_pos && OVERFLOW_POS == m_write_pos)
    {
        m_read_pos = 0;
        m_write_pos = 0;

        return 0;
    }
    // Should not happen.
    //else if (OVERFLOW_POS == m_read_pos && OVERFLOW_POS != m_write_pos)
        //return 0;

    /*
     * else:
     * OVERFLOW_POS != m_read_pos && OVERFLOW_POS == m_write_pos
     * or
     * OVERFLOW_POS != m_read_pos && OVERFLOW_POS != m_write_pos
     */

    int bytes_to_move = data_size();

    if (0 == m_read_pos)
    {
        if (bytes_to_move >= total_size())
            m_write_pos = OVERFLOW_POS;

        return 0;
    }

    cdebug("read pos = %d, write pos = %d, moving %d bytes data to header ...\n",
        m_read_pos, m_write_pos, bytes_to_move);
    memmove(m_data, ((char *)m_data) + m_read_pos, bytes_to_move);
    m_read_pos = 0;
    m_write_pos = bytes_to_move;

    return bytes_to_move;
}

void sequential_buffer::reset(bool zero_all_data/* = false*/)
{
    m_write_pos = 0;
    m_read_pos = 0;
    if (zero_all_data)
        memset(m_data, 0, m_total_size);
}

int sequential_buffer::innerly_create(int size)
{
    if (size <= 0)
        return CA_RET(INVALID_PARAM_VALUE);

    if (size < MIN_BUF_SIZE)
        size = MIN_BUF_SIZE;

    if (m_total_size == size && nullptr != m_data)
        return CA_RET_OK;

    clear();

    if (nullptr == (m_data = calloc(size, sizeof(char))))
        return CA_RET(MEMORY_ALLOC_FAILED);

    m_total_size = size;

    return CA_RET_OK;
}

int sequential_buffer::copy_from(const sequential_buffer& src)
{
    bool size_changed = (this->total_size() != src.total_size());

    if (size_changed)
        clear();

    int ret = size_changed ? innerly_create(src.total_size()) : CA_RET_OK;

    if (CA_RET_OK != ret)
        return ret;

    m_write_pos = src.m_write_pos;
    m_read_pos = src.m_read_pos;

    int data_size = this->data_size();
    int read_pos = (OVERFLOW_POS != m_read_pos) ? m_read_pos : 0;

    if (data_size > 0)
        memcpy(((char*)m_data) + read_pos, ((char*)src.m_data) + read_pos, data_size);

    return CA_RET_OK;
}

void sequential_buffer::init(void)
{
    m_data = nullptr;
    m_total_size = 0;
    m_read_pos = 0;
    m_write_pos = 0;
}

void sequential_buffer::clear(void)
{
    if (nullptr != m_data)
    {
        free(m_data);
        m_data = nullptr;
    }
    m_total_size = 0;
    m_read_pos = 0;
    m_write_pos = 0;
}

CA_LIB_NAMESPACE_END
