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
 * protocol_common.h
 *
 *  Created on: 2015-06-03
 *      Author: wenxiongchang
 * Description: common stuff for protocol handling
 */

#ifndef __PROTOCOL_COMMON_H__
#define __PROTOCOL_COMMON_H__

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#include <typeinfo>

#include <cpp_assistant/ca_full.h>
#include <mutable_customization.h> // A header file defined by user.

#include "public_protocols.pb.h"

namespace cafw
{

typedef ::google::protobuf::Message     msg_base;

typedef MinimalBody                     OldHeartbeatReq;
typedef MinimalBody                     OldHeartbeatResp;

typedef OldIdentityReportReq            IdentityReportReq;
typedef MinimalBody                     IdentityReportResp;

typedef MinimalBody                     ReqBodyPrefix;
typedef MinimalBody                     RespBodyPrefix;
typedef MinimalBody                     UnifiedBodyPrefix;

#ifndef SID_LEN
#define SID_LEN                         32
#endif

#define DECLARE_AND_CAST(base_ptr, derived_ptr, derived_type)                       \
    derived_type *derived_ptr = dynamic_cast<derived_type *>(base_ptr)

#ifndef SET_REQ_PREFIX
#define SET_REQ_PREFIX(req_body, sid, route_id)                                      do{\
    (req_body).set_session_id(sid, SID_LEN); \
}while (0)
#endif

#ifndef OUTPUT_REQ_PREFIX
#define OUTPUT_REQ_PREFIX(req_body, logger_prefix)                                  do{\
    logger_prefix##LOG_INFO("Request body prefix: session_id[%s]\n", \
        (req_body).session_id().c_str()); \
}while (0)
#endif

#define G_OUTPUT_REQ_PREFIX(req_body)                                               OUTPUT_REQ_PREFIX(req_body, G)
#define T_OUTPUT_REQ_PREFIX(req_body)                                               OUTPUT_REQ_PREFIX(req_body, T)

#ifndef SET_RESP_PREFIX
#define SET_RESP_PREFIX(req_body, resp_body, retcode)                               do{\
    (resp_body).set_session_id((req_body).session_id().c_str(), SID_LEN); \
}while (0)
#endif

#ifndef OUTPUT_RESP_PREFIX
#define OUTPUT_RESP_PREFIX(resp_body, logger_prefix)                                do{\
    logger_prefix##_LOG_INFO("Response body prefix: session_id[%s]\n", \
        (resp_body).session_id().c_str()); \
}while (0)
#endif

#define G_OUTPUT_RESP_PREFIX(resp_body)                                             OUTPUT_RESP_PREFIX(resp_body, G)
#define T_OUTPUT_RESP_PREFIX(resp_body)                                             OUTPUT_RESP_PREFIX(resp_body, T)

#ifndef CMD_HEARTBEAT_REQ
#define CMD_HEARTBEAT_REQ                                   0x00000000
#endif

#ifndef CMD_HEARTBEAT_RESP
#define CMD_HEARTBEAT_RESP                                  0x00000001
#endif

#ifndef CMD_IDENTITY_REPORT_REQ
#define CMD_IDENTITY_REPORT_REQ                             0x00000002
#endif

#ifndef CMD_IDENTITY_REPORT_RESP
#define CMD_IDENTITY_REPORT_RESP                            0x00000003
#endif

#ifndef CMD_UNUSED
#define CMD_UNUSED                                          0x11111111
#endif

#ifndef PROTO_RET_SUCCESS
#define PROTO_RET_SUCCESS                                   888888
#endif

#ifndef PROTO_RET_UNKNOWN_ERROR
#define PROTO_RET_UNKNOWN_ERROR                             0
#endif

#ifndef PROTO_RET_PACKET_PARSE_ERROR
#define PROTO_RET_PACKET_PARSE_ERROR                        444444
#endif

/*#ifndef PROTO_RET_PACKET_ASSEMBLE_ERROR
#define PROTO_RET_PACKET_ASSEMBLE_ERROR                     90002
#endif*/

template <typename Int>
static Int get_proto_header_field(const void *raw_buf,
    const int field_offset,
    const Int abnormal_value)
{
    if (NULL == raw_buf)
        return abnormal_value;

    Int value = abnormal_value;
    size_t value_size = sizeof(Int);

    memcpy(&value, (char *)raw_buf + field_offset, value_size);

    if (sizeof(int8_t) == value_size)
        return value;
    else if (sizeof(int16_t) == value_size)
        return (Int)ntohs(value);
    else if (sizeof(int32_t) == value_size)
        return (Int)ntohl(value);
    else
        return (Int)cal::sys::ntohl64(value);
}

enum
{
    ERR_INVALID_LENGTH = 0x00000000,
    ERR_INVALID_COMMAND = 0x00000000
};

#define GLOG_PROTO_FIELD(field_owner, field_name, fmt, ...)                          do{\
    if ((field_owner).has_##field_name()) \
    GLOG_INFO(fmt, ##__VA_ARGS__); \
}while(0)

#ifndef PROTO_HEADER_BASICS_SIZE
#define PROTO_HEADER_BASICS_SIZE                            24
#endif

#ifndef PROTO_HEADER_EXTENSIONS_SIZE
#define PROTO_HEADER_EXTENSIONS_SIZE                        0
#endif

#ifndef PROTO_HEADER_SIZE
#define PROTO_HEADER_SIZE                                   (PROTO_HEADER_BASICS_SIZE + PROTO_HEADER_EXTENSIONS_SIZE)
#endif

typedef struct proto_header_t
{
    uint32_t length; // total length = fixed header length + mutable body length
    // TODO: ifdef LONG_ROUTE_ID: uint64_t else: uint32_t
    uint64_t route_id; // for route searching, packet dispatching, etc.
    uint32_t command;
    int32_t packet_number; // MSB is the end flag: 0 for not ended, 1 for ended. abs(packet_number) is the actual number.
    uint32_t error_code;
    char extension_padding[PROTO_HEADER_EXTENSIONS_SIZE];
}proto_header_t;

#ifndef HEADER_OFFSET_LENGTH
#define HEADER_OFFSET_LENGTH                                0
#endif

#ifndef HEADER_OFFSET_ROUTE_ID
#define HEADER_OFFSET_ROUTE_ID                              (HEADER_OFFSET_LENGTH + sizeof(uint32_t))
#endif

#ifndef HEADER_OFFSET_COMMAND
#define HEADER_OFFSET_COMMAND                               (HEADER_OFFSET_ROUTE_ID + sizeof(uint64_t))
#endif

#ifndef HEADER_OFFSET_PACKET_NUM
#define HEADER_OFFSET_PACKET_NUM                            (HEADER_OFFSET_COMMAND + sizeof(uint32_t))
#endif

#ifndef HEADER_OFFSET_ERR_CODE
#define HEADER_OFFSET_ERR_CODE                              (HEADER_OFFSET_PACKET_NUM + sizeof(int32_t))
#endif

#ifndef HEADER_OFFSET_EXTENSIONS
#define HEADER_OFFSET_EXTENSIONS                            (HEADER_OFFSET_ERR_CODE + sizeof(uint32_t))
#endif

inline void fill_proto_header(proto_header_t &header,
    const uint32_t body_len,
    const uint32_t cmd,
    const uint32_t errcode,
    const uint64_t route_id = 0,
    const int32_t packet_num = 1,
    const bool is_final_packet = true,
    const char* extensions = NULL)
{
    header.length = PROTO_HEADER_SIZE + body_len;
    header.command = cmd;
    header.packet_number = (is_final_packet) ? (-(packet_num)) : (packet_num);
    header.route_id = route_id;
    header.error_code = errcode;
    // TODO: how to fill extensions?
}

#define CALC_BODY_LEN(total_len)                ((total_len) - PROTO_HEADER_SIZE)
#define GET_HEADER_ADDR(buf)                    ((void *)buf)
#define GET_BODY_ADDR(buf)                      ((void *)((char *)buf + PROTO_HEADER_SIZE))

enum PacketEndFlag
{
    PACKET_NOT_ENDED = 0,
    PACKET_ENDED = 1
};

inline uint32_t get_proto_length(const void *raw_buf)
{
    return get_proto_header_field(raw_buf, HEADER_OFFSET_LENGTH, (uint32_t)ERR_INVALID_LENGTH);
}

inline uint32_t get_proto_command(const void *raw_buf)
{
    return get_proto_header_field(raw_buf, HEADER_OFFSET_COMMAND, (uint32_t)ERR_INVALID_COMMAND);
}

inline uint32_t get_proto_packet_number(const void *raw_buf)
{
    int32_t value = get_proto_header_field(raw_buf, HEADER_OFFSET_PACKET_NUM, (int32_t)-1);

    return (value < 0) ? (-value) : value;
}

inline bool is_final_packet(const void *raw_proto_buf)
{
    int32_t value = get_proto_header_field(raw_proto_buf, HEADER_OFFSET_PACKET_NUM, (int32_t)-1);

    return (value < 0);
}

inline uint64_t get_proto_route_id(const void *raw_buf)
{
    return get_proto_header_field(raw_buf, HEADER_OFFSET_ROUTE_ID, (uint64_t)0);
}

inline uint32_t get_proto_error_code(const void *raw_buf)
{
    return get_proto_header_field(raw_buf, HEADER_OFFSET_ERR_CODE, (uint32_t)0);
}

//int proto_precheck(const int inlen, const void* inbuf, int &outlen, void* outbuf, int &retcode);

//int validate_protocol_header(void *buf, int len, uint32_t cmd);

int parse_header(const void *inbuf, proto_header_t *result);
int assemble_header(const proto_header_t *src, void *outbuf);

inline bool proto_is_request(const uint32_t cmd)
{
    return (0 == cmd % 2);
}

inline bool proto_is_heartbeat(const uint32_t cmd)
{
    return (CMD_HEARTBEAT_REQ == cmd ||
        CMD_HEARTBEAT_RESP == cmd);
}

int get_unified_proto_prefix_length(void);
int get_request_proto_prefix_length(void);
int get_response_proto_prefix_length(void);

int extract_session_id(const void *raw_buf, char *result, bool is_req = true);

int serialize_to_buffer(const uint32_t out_cmd,
    const msg_base *out_body,
    const void *in_header_buf,
    void *out_buf,
    int &out_len,
    const uint32_t errcode = PROTO_RET_SUCCESS,
    const bool is_final_fragment = true,
    const int32_t packet_num = 1);

int send_lite_packet(const int fd,
    const uint32_t cmd,
    const uint32_t errcode,
    const msg_base *msg_body,
    const uint64_t route_id = 0,
    const bool is_final_fragment = true,
    const int32_t packet_num = 1);

// NOTE: This function should be executed by supervisor thread only.
int send_lite_packet(const char *node_type,
    const int max_node_count,
    const uint32_t cmd,
    const uint32_t errcode,
    const msg_base *msg_body,
    const bool to_all = true,
    const int policy = 0,
    const uint64_t route_id = 0,
    const bool is_final_fragment = true,
    const int32_t packet_num = 1);

int send_heartbeat_request(const int fd);
int send_identity_report_request(const int fd);

// Contents commented below should be put into user's code.
/*#define PRECHECK_BEFORE_PRINTING(out_body_type, out_body_var, msg_ptr, msg_len, cmd) \
    out_body_type out_body_var; \
    if (RET_OK != validate_protocol_header(msg_ptr, msg_len, cmd)) \
        return; \
    uint32_t total_len = get_proto_length(msg_ptr); \
    int body_len = CALC_BODY_LEN(total_len); \
    out_body_var.ParseFromArray(GET_BODY_ADDR(msg_ptr), body_len)

// usually used in packet validation functions
#define CHK_FIELD_EXISTENCE(field_owner, field_name, retcode_var)                   do{\
    if (!(field_owner).has_##field_name()) \
    {\
        LOG_ERROR_V("missing "#field_name"\n"); \
        (retcode_var) = PROTO_RET_MISSING_PROTO_FIELD; \
        return false; \
    }\
}while(0)

// usually used in packet validation functions
#define CHK_INT_FIELD_RANGE(field_var, min, max, rc_var, rc_val)                    do{\
    if (field_var < min || field_var > max) \
    {\
        LOG_ERROR_V("value of "#field_var" is beyond range[%d, %d]\n", min, max); \
        (rc_var) = (rc_val); \
        return false; \
    }\
}while(0)*/

const char *desc_of_end_flag(int src_value);
/*const char *desc_of_operation_type(int src_value);
const char *desc_of_effect_status(int src_value);*/

} // namespace cafw

extern bool proto_uses_general_prefix(const uint32_t cmd);

extern int make_session_id(const void *condition, const int sid_holder_size, char *sid_result);

extern bool session_exists(const char *sid);
extern int fetch_session_info(const char *sid, void* outbuf, int &outlen);
extern int save_session_info(const struct cal::net_connection *src_conn, const void* buf, int buflen, bool commit_now = false);
extern int save_session_info(const struct cal::net_connection *src_conn, const cafw::msg_base *body, const cafw::proto_header_t &header, bool commit_now = false);
extern int update_session_info(const void *buf, int buflen, bool commit_now = false);
extern int update_session_info(const cafw::msg_base *body, const cafw::proto_header_t &header, bool commit_now = false);
extern void clean_expired_sessions(void);

extern bool message_is_time_consuming(const uint32_t cmd);

extern bool do_security_check_to_packet(const void* packet);

#endif /* __PROTOCOL_COMMON_H__ */
