/*
 * Copyright (c) 2016-2018, Wen Xiongchang <udc577 at 126 dot com>
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
 * packet_processor.h
 *
 *  Created on: 2015-06-02
 *      Author: wenxiongchang
 * Description: for processing network packets
 */

#ifndef __PACKET_PROCESSOR_H__
#define __PACKET_PROCESSOR_H__

#include "handler_component_definitions.h"

namespace cafw
{

class message_cache;

class packet_processor
{
/* ===================================
 * constructors:
 * =================================== */
public:
    packet_processor();

/* ===================================
 * copy control:
 * =================================== */
private:
    packet_processor(const packet_processor& src);
    packet_processor& operator=(const packet_processor& src);

/* ===================================
 * destructor:
 * =================================== */
public:
    ~packet_processor();

/* ===================================
 * types:
 * =================================== */
public:

/* ===================================
 * abilities:
 * =================================== */
public:
    int build_component_map(void);

    int process(const struct calns::net_connection *input_conn,
        int &handled_len,
        struct calns::net_connection **mutable_output_conn,
        int &output_len);

    static void print_supported_commands(void);

    static int get_current_max_packet_length(void);

    static void update_max_packet_length(int len);

/* ===================================
 * attributes:
 * =================================== */
public:
    message_cache* get_message_cache(void) const
    {
        return m_message_cache;
    }

/* ===================================
 * status:
 * =================================== */
public:
    bool is_available(void);
    bool is_ready(void);


/* ===================================
 * operators:
 * =================================== */
public:

/* ===================================
 * private methods:
 * =================================== */
protected:
    int __inner_init(void);
    void __clear(void);

    int single_operator_general_flow(const struct calns::net_connection *input_conn,
        const int input_len,
        handler_component &component,
        struct calns::net_connection **mutable_output_conn,
        int &output_len);

    int dispacher_general_flow(const struct calns::net_connection *input_conn,
        const int input_len,
        struct calns::net_connection **mutable_output_conn,
        int &output_len);

    int diagnose_connection(const struct calns::net_connection *input_conn,
        const int input_len,
        struct calns::net_connection **mutable_output_conn,
        int &output_len);

/* ===================================
 * data:
 * =================================== */
protected:
    message_cache *m_message_cache;
    component_map *m_component_map;
    std::map<std::string, int64_t> *m_timestamps_when_pkts_incomplete;
    static int m_max_packet_length;
};

}

#endif /* __PACKET_PROCESSOR_H__ */
