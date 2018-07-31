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
 * handler_component_definitions.h
 *
 *  Created on: 2015-06-08
 *      Author: wenxiongchang
 * Description: handler component definitions and declarations
 */

#ifndef __HANDLER_COMPONENT_DEFINITIONS_H__
#define __HANDLER_COMPONENT_DEFINITIONS_H__

#include <stddef.h>

#include <map>

#include "protocol_common.h"

namespace cafw
{

typedef int (*group_fragment_func)(const msg_base *fragment, msg_base *whole);

typedef bool (*validate_input_packet_func)(msg_base *parsed_body, int &retcode);

typedef int (*business_func)(
    const calns::net_connection *in_conn,
    const cafw::msg_base* in_body,
    calns::net_connection **out_conn,
    cafw::msg_base* out_body,
    int &retcode);

typedef msg_base* (*body_container_alloc_func)(void);

typedef void (*assemble_output_packet_func)(const void *original_msg, int retcode, msg_base *in, msg_base *out);

typedef struct handler_component
{
    int32_t in_cmd;
    int32_t out_cmd;
    const char *description;
    bool filters_repeated_session;
    bool has_multi_fragments;
    msg_base *partial_in_body;
    msg_base *whole_in_body;
    msg_base *out_body;
    group_fragment_func group_fragments;
    validate_input_packet_func in_packet_is_ok;
    business_func do_business;
    body_container_alloc_func alloc_body_container;
    assemble_output_packet_func assemble_out_packet;
}handler_component;

typedef std::map<uint32_t, handler_component> component_map;

#define GROUP_PACKET_FUNC(name)                 group_##name##_fragments

#define VALIDATE_PACKET_FUNC(name)              name##_in_packet_is_ok

#define BUSINESS_FUNC(name)                     do_##name##_business

#define ALLOC_PACKET_CONTAINER_FUNC(name)       alloc_##name##_body_container

#define ASSEMBLE_OUT_PACKET_FUNC(name)          assemble_##name##_out_packet

#define SET_GROUP_FUNC(name, func_ptr)          cafw::group_fragment_func GROUP_PACKET_FUNC(name) = func_ptr
#define SET_GROUP_FUNC_TO_NULL(name)            SET_GROUP_FUNC(name, NULL)

#define SET_VALIDATE_FUNC(name, func_ptr)       cafw::validate_input_packet_func VALIDATE_PACKET_FUNC(name) = func_ptr
#define SET_VALIDATE_FUNC_TO_NULL(name)         SET_VALIDATE_FUNC(name, NULL)

#define SET_BUSINESS_FUNC(name, func_ptr)       cafw::business_func BUSINESS_FUNC(name) = func_ptr
#define SET_BUSINESS_FUNC_TO_NULL(name)         SET_BUSINESS_FUNC(name, NULL)

#define SET_ALLOC_FUNC(name, func_ptr)          cafw::body_container_alloc_func ALLOC_PACKET_CONTAINER_FUNC(name) = func_ptr
#define SET_ALLOC_FUNC_TO_NULL(name)            SET_ALLOC_FUNC(name, NULL)

#ifndef USE_JSON_MSG

#define DEFINE_ALLOC_FUNC(name, container_type) \
    cafw::msg_base* ALLOC_PACKET_CONTAINER_FUNC(name)(void) \
    {\
        return new container_type; \
    }

#else

//DECLARE_ALLOC_FUNC(json); // compile error
msg_base* alloc_json_body_container(void);

#endif // #ifndef USE_JSON_MSG

#define SET_ASSEMBLE_OUT_FUNC(name, func_ptr)   cafw::assemble_output_packet_func ASSEMBLE_OUT_PACKET_FUNC(name) = func_ptr
#define SET_ASSEMBLE_OUT_FUNC_TO_NULL(name)     SET_ASSEMBLE_OUT_FUNC(name, NULL)

#ifndef USE_JSON_MSG

#define DEFINE_DEFAULT_ASSEMBLE_OUT_FUNC(name, in_type, out_type) \
    DECLARE_ASSEMBLE_OUT_FUNC(name) \
    {\
        DECLARE_AND_CAST(in_body, req, in_type); \
        DECLARE_AND_CAST(out_body, resp, out_type); \
        SET_RESP_PREFIX((*req), (*resp), retcode); \
    }

#else

//DECLARE_ASSEMBLE_OUT_FUNC(minimal); // compile error
void assemble_minimal_out_packet(const void *original_msg, int retcode, msg_base *in_body, msg_base *out_body);

#endif // #ifndef USE_JSON_MSG


#define DECLARE_GROUP_FUNC(name)                int GROUP_PACKET_FUNC(name)(const cafw::msg_base *fragment, cafw::msg_base *whole)

#define DECLARE_VALIDATE_FUNC(name)             bool VALIDATE_PACKET_FUNC(name)(cafw::msg_base *parsed_body, int &retcode)

#define DECLARE_BUSINESS_FUNC(name)             int BUSINESS_FUNC(name)(const calns::net_connection *in_conn, const cafw::msg_base* in_body, \
    calns::net_connection **out_conn, cafw::msg_base* out_body, int &retcode)

#define DECLARE_ALLOC_FUNC(name)                cafw::msg_base* ALLOC_PACKET_CONTAINER_FUNC(name)(void)

#define DECLARE_ASSEMBLE_OUT_FUNC(name)         void ASSEMBLE_OUT_PACKET_FUNC(name)(const void *original_msg, int retcode, \
    cafw::msg_base *in_body, cafw::msg_base *out_body)

#define CALL_GROUP_FUNC(name, fragment, whole)                                          \
    GROUP_PACKET_FUNC(name)ts(fragment, whole)

#define CALL_VALIDATE_FUNC(name, parsed_body, retcode)                                  \
    VALIDATE_PACKET_FUNC(name)(parsed_body, retcode)

#define CALL_BUSINESS_FUNC(name, in_buf, in_body, out_buf, out_body, retcode)           \
    BUSINESS_FUNC(name)(in_buf, in_body, out_buf, out_body, retcode)

#define CALL_ALLOC_FUNC(name)                                                           \
    ALLOC_PACKET_CONTAINER_FUNC(name)()

#define CALL_ASSEMBLE_OUT_FUNC(name, in_buf, retcode, in_body, out_body)                \
    ASSEMBLE_OUT_PACKET_FUNC(name)(in_buf, retcode, in_body, out_body)

#define SET_HANDLER_FUNCS_BY_NAME(name)      GROUP_PACKET_FUNC(name), \
    VALIDATE_PACKET_FUNC(name), \
    BUSINESS_FUNC(name), \
    ALLOC_PACKET_CONTAINER_FUNC(name), \
    ASSEMBLE_OUT_PACKET_FUNC(name)

#define SET_HANDLER_FUNCS_ONE_BY_ONE(name, has_group_func, has_validation_func,        \
    has_assemble_func) \
    has_group_func ? GROUP_PACKET_FUNC(name) : NULL, \
    has_validation_func ? VALIDATE_PACKET_FUNC(name) : NULL, \
    BUSINESS_FUNC(name), \
    has_group_func ? ALLOC_PACKET_CONTAINER_FUNC(name) : NULL, \
    has_assemble_func ? ASSEMBLE_OUT_PACKET_FUNC(name) : NULL

#define SET_ALL_HANDLER_FUNCS_TO_NULL()                                                \
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL

#ifndef USE_JSON_MSG

#define SET_BODY_CONTAINERS(has_multi_fragments, in_class, has_output, out_class)      \
    has_multi_fragments, \
    (has_multi_fragments) ? (new in_class) : NULL, \
    (!has_multi_fragments) ? (new in_class) : NULL, \
    (has_output) ? (new out_class) : NULL

#else

#define SET_BODY_CONTAINERS(has_multi_fragments, has_output)      \
    has_multi_fragments, \
    (has_multi_fragments) ? (new Json::Value) : NULL, \
    (!has_multi_fragments) ? (new Json::Value) : NULL, \
    (has_output) ? (new Json::Value) : NULL

#endif // #ifndef USE_JSON_MSG

#define SET_ALL_BODY_CONTAINERS_TO_NULL()                                              \
    NULL, NULL, NULL

#define FILTERS_REPEATED_SESSION                    true
#define NO_FILTER_REPEATED_SESSION                  false

#define HAS_MULTI_FRAGMENTS                         true
#define NO_MULTI_FRAGMENTS                          false

#define HAS_OUTPUT                                  true
#define NO_OUTPUT                                   false

#define HAS_GROUP_FUNC                              true
#define NO_GROUP_FUNC                               false

#define HAS_VALIDATION_FUNC                         true
#define NO_VALIDATION_FUNC                          false

#define NO_ASSEMBLE_OUT_FUNC                        false
#define HAS_ASSEMBLE_OUT_FUNC                       true

#define NEEDS_DB_COMMIT                             true
#define NO_DB_COMMIT                                false

} // namespace cafw

#endif /* __HANDLER_COMPONENT_DEFINITIONS_H__ */
