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

/*
 * sequential_buffer.h
 *
 *  Created on: 2016-10-28
 *      Author: wenxiongchang
 * Description: Sequential buffer.
 */

#ifndef __CPP_ASSISTANT_SEQUENTIAL_BUFFER_H__
#define __CPP_ASSISTANT_SEQUENTIAL_BUFFER_H__

#include "base/ca_inner_necessities.h"

CA_LIB_NAMESPACE_BEGIN

/*
 * Constants of SeqBuf[N]:
    position range:     [0, N - 1]
    invalid position:   -1 or N (depending on implementation)

Prerequisites:
    1) read position: the first position for existing data.
    2) write position: the first position for new data writing.
    3) read position must be before write position,
       except that one of them is OVERFLOW_PTR.
    4) both read position and write position should be 0 at the very beginning.


Variables of SeqBuf[N]:
    -------------------------------------------------------------------------------------------
    Condition r below has two meanings depending on the contexts:
        1) actual read position;
        or
        2) the logical result of whether read position is valid or not.
    w: Like definitions of r.
    -------------------------------------------------------------------------------------------
    (r, w)   |   data_size,  free_size,  is_empty,   is_full,    R_PTR,          W_PTR
    -------------------------------------------------------------------------------------------
    (0, 0)   |   0,          N,          true,       false,      OVERFLOW_PTR,   OVERFLOW_PTR
    (0, 1)   |   /,          /,          /,          /,          /,              /
    (1, 0)   |   N-r,        r,          false,      true/false, start_ptr+r,    OVERFLOW_PTR
    (1, 1)   |   w-r,        N-w+r,      true/false, false,      start_ptr+r,    start_ptr+w
 */

class sequential_buffer
{
/* ===================================
 * constructors:
 * =================================== */
public:
    sequential_buffer();
    explicit sequential_buffer(int size);

/* ===================================
 * copy control:
 * =================================== */
public:
    sequential_buffer(const sequential_buffer& src);
    sequential_buffer& operator=(const sequential_buffer& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~sequential_buffer();

/* ===================================
 * types:
 * =================================== */
public:
    enum enum_bufsize
    {
        MIN_BUF_SIZE = 1024,
        DEFAULT_BUF_SIZE = 1024 * 4,
        //MAX_BUF_SIZE = 1024 * 1024 * 100 // Maximum size is decided by upper layer.
    };

    enum
    {
        OVERFLOW_POS = -1
    };

/* ===================================
 * abilities:
 * =================================== */
public:
    // Creates a buffer whose total size is @size or MIN_BUF_SIZE.
    int create(int size);

    // Destroys the buffer, all resources and structures within it will be released,
    // and Create() afterwards is allowed.
    // If you just want to clear data and status values within buffer, use Reset().
    void destroy(void);

    // Resizes the buffer.
    // Returns new size on success, or 0 if size is unchanged, or a negative number on failure.
    int resize(int new_size);

    /*
     * Reads/Writes data from/into buffer, and read/write pointer also moves.
     * Returns bytes read/written on success, or a negative number on failure.
     */
    int read(const int len, void *data);
    int write(const int len, const void *data);

    /*
     * Gets/Moves read/write pointer. It's useful when you want to operate the
     * buffer directly and avoid one more time of copying in your program.
     * Returns the new read/write pointer, or how many bytes
     * that the read/write pointer just actually moved.
     *
     * NOTE: Read/Write pointer is only allowed to move forward, not to move backward,
     *       unless the target position is OVERFLOW_POS!
     */
    void *get_read_pointer(void);
    int move_read_pointer(const int offset);
    void *get_write_pointer(void);
    int move_write_pointer(const int offset);

    // Moves data to header of buffer in order to make more continuous space for new data.
    // Returns bytes moved.
    int move_data_to_header(void);

    // Resets all status values(read pointer, write pointer, etc.),
    // and data will be set 0 if @zero_all_data is true.
    // If you want to release the buffer completely, use Destroy().
    void reset(bool zero_all_data = false);

/* ===================================
 * attributes:
 * =================================== */
public:
    DEFINE_CLASS_NAME_FUNC()

    inline void *data(void) const
    {
        return m_data;
    }

    inline const int data_size(void) const
    {
        // The most common case.
        if (OVERFLOW_POS != m_read_pos && OVERFLOW_POS != m_write_pos)
            return m_write_pos - m_read_pos;

        if (OVERFLOW_POS != m_read_pos && OVERFLOW_POS == m_write_pos)
            return total_size() - m_read_pos;

        // We would not let this happen!!!
        //if (OVERFLOW_POS == m_read_pos && OVERFLOW_POS != m_write_pos)
        //  return m_write_pos;

        //if (OVERFLOW_POS == m_read_pos && OVERFLOW_POS == m_write_pos)
        return 0;
    }

    inline const int total_size(void) const
    {
        return (m_total_size >= 0) ? m_total_size : 0;
    }

    inline const int free_size(void) const
    {
        int free_size = total_size() - data_size();

        return (free_size >= 0) ? free_size : 0;
    }

    inline const int read_position(void) const
    {
        return m_read_pos;
    }

    inline const int write_position(void) const
    {
        return m_write_pos;
    }

/* ===================================
 * status:
 * =================================== */
public:
    inline const bool empty(void) const
    {
        return data_size() <= 0;
    }

    inline const bool full(void) const
    {
        return (data_size() >= total_size());
    }

    inline const bool has_overflown_pointer(void) const
    {
        return (OVERFLOW_POS == m_read_pos
            || m_read_pos >= m_total_size
            || OVERFLOW_POS == m_write_pos
            || m_write_pos >= m_total_size);
    }

/* ===================================
 * operators:
 * =================================== */
public:

/* ===================================
 * private methods:
 * =================================== */
protected:
    int innerly_create(int size);
    int copy_from(const sequential_buffer& src);
    void init(void);
    void clear(void);

/* ===================================
 * data:
 * =================================== */
protected:
    DECLARE_CLASS_NAME_VAR();
    void *m_data;
    int m_total_size;
    int m_read_pos;
    int m_write_pos;

public:
    static const void* OVERFLOW_PTR;
};

typedef sequential_buffer seqbuf;

CA_LIB_NAMESPACE_END

#endif // __CPP_ASSISTANT_SEQUENTIAL_BUFFER_H__

