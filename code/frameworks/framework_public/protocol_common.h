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

#include "base/all.h"
#include "mutable_customization.h" // A header file defined by user.

#ifndef USE_JSON_MSG
#include "public_protocols.pb.h"
#else
#include "json/json.h"
#endif

namespace cafw
{

#ifndef SID_LEN
#define SID_LEN                         32
#endif

#define DECLARE_AND_CAST(base_ptr, derived_ptr, derived_type)                       \
    derived_type *derived_ptr = dynamic_cast<derived_type *>(base_ptr)

#ifndef USE_JSON_MSG

typedef ::google::protobuf::Message     msg_base;

typedef MinimalBody                     OldHeartbeatReq;
typedef MinimalBody                     OldHeartbeatResp;

typedef OldIdentityReportReq            IdentityReportReq;
typedef MinimalBody                     IdentityReportResp;

typedef MinimalBody                     ReqBodyPrefix;
typedef MinimalBody                     RespBodyPrefix;
typedef MinimalBody                     UnifiedBodyPrefix;

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

#define GLOG_PROTO_FIELD(field_owner, field_name, fmt, ...)                          do{\
    if ((field_owner).has_##field_name()) \
        RLOGF(I, fmt, ##__VA_ARGS__); \
}while(0)

#else

typedef Json::Value                     msg_base;

#define SID_KEY_STR                     "session_id"
#define SERVER_TYPE_KEY_STR             "server_type"
#define SERVER_NAME_KEY_STR             "server_name"

#ifndef SET_REQ_PREFIX
#define SET_REQ_PREFIX(req_body, sid, route_id)                                      do{\
    (req_body)[SID_KEY_STR] = sid; \
}while (0)
#endif

#ifndef OUTPUT_REQ_PREFIX
#define OUTPUT_REQ_PREFIX(req_body, logger_prefix)                                  do{\
    const Json::Value &_sid_json_value = (req_body)[SID_KEY_STR]; \
    if (!_sid_json_value.empty()) \
        logger_prefix##LOG_INFO("Request body prefix: session_id[%s]\n", \
            _sid_json_value.asCString()); \
}while (0)
#endif

#ifndef SET_RESP_PREFIX
#define SET_RESP_PREFIX(req_body, resp_body, retcode)                               do{\
    (resp_body)[SID_KEY_STR] = (req_body)[SID_KEY_STR].asString(); \
}while (0)
#endif

#ifndef OUTPUT_RESP_PREFIX
#define OUTPUT_RESP_PREFIX(resp_body, logger_prefix)                                do{\
    const Json::Value &_sid_json_value = (resp_body)[SID_KEY_STR]; \
    if (!_sid_json_value.empty()) \
        logger_prefix##_LOG_INFO("Response body prefix: session_id[%s]\n", \
            _sid_json_value.asCString()); \
}while (0)
#endif

#define GLOG_PROTO_FIELD(field_owner, field_name, fmt, ...)                          do{\
    if (!(field_owner)[#field_name].empty()) \
        RLOGF(I, fmt, ##__VA_ARGS__); \
}while(0)

#endif // #ifndef USE_JSON_MSG

#define G_OUTPUT_REQ_PREFIX(req_body)                                               OUTPUT_REQ_PREFIX(req_body, G)
#define T_OUTPUT_REQ_PREFIX(req_body)                                               OUTPUT_REQ_PREFIX(req_body, T)

#define G_OUTPUT_RESP_PREFIX(resp_body)                                             OUTPUT_RESP_PREFIX(resp_body, G)
#define T_OUTPUT_RESP_PREFIX(resp_body)                                             OUTPUT_RESP_PREFIX(resp_body, T)

enum
{
    INVALID_COMMAND = 0x44444444
};

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
#define CMD_UNUSED                                          cafw::INVALID_COMMAND
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

enum
{
    INVALID_PACKET_LENGTH = 0
};

#ifndef PROTO_HEADER_EXTENSIONS_SIZE
#define PROTO_HEADER_EXTENSIONS_SIZE                        0
#endif

typedef struct proto_header_t
{
    /*
     * Since some languages do not support unsigned integer type,
     * therefore all integer type fields should be defined as signed.
     *
     * NOTE: The header should be 4-byte aligned.
     */

    // total length = fixed header length + mutable body length
    int32_t length;

    // For route searching, packet dispatching, etc.
    int64_t route_id;

    // For determining what a packet is.
    // For example, 0x00000000 indicates the packet is a heart-beat request.
    int32_t command;

    // A set of flags. A bit or some bits mean(s) something:
    // bit 0: packet end flag: 0 for not ended, 1 for ended.
    // other bits: reserved for future use.
    int16_t flag_bits;

    // The number of the current packet, starting from 1.
    int16_t packet_number;

    // For determining the error reason if an error occurs during packet handling.
    // For example, 888888 means successful, 444444 means packet parse exception.
    int32_t error_code;

    // This extension field is totally defined by user.
    char extension_padding[PROTO_HEADER_EXTENSIONS_SIZE];
}proto_header_t;

#define HEADER_OFFSET_LENGTH                                0

#define HEADER_OFFSET_ROUTE_ID                              (HEADER_OFFSET_LENGTH + sizeof(int32_t))

#define HEADER_OFFSET_COMMAND                               (HEADER_OFFSET_ROUTE_ID + sizeof(int64_t))

#define HEADER_OFFSET_FLAG_BITS                             (HEADER_OFFSET_COMMAND + sizeof(int32_t))

#define HEADER_OFFSET_PACKET_NUM                            (HEADER_OFFSET_FLAG_BITS + sizeof(int16_t))

#define HEADER_OFFSET_ERR_CODE                              (HEADER_OFFSET_PACKET_NUM + sizeof(int16_t))

#define HEADER_OFFSET_EXTENSIONS                            (HEADER_OFFSET_ERR_CODE + sizeof(int32_t))

#ifndef PROTO_HEADER_BASICS_SIZE
#define PROTO_HEADER_BASICS_SIZE                            HEADER_OFFSET_EXTENSIONS
#endif

#define PROTO_HEADER_SIZE                                   (PROTO_HEADER_BASICS_SIZE + PROTO_HEADER_EXTENSIONS_SIZE)

inline void fill_proto_header(proto_header_t &header,
    const int32_t body_len,
    const int32_t cmd,
    const int32_t errcode,
    const int64_t route_id = 0,
    const int16_t packet_num = 1,
    const int16_t flag_bits = 1,
    const char* extensions = NULL)
{
    header.length = PROTO_HEADER_SIZE + body_len;
    header.route_id = route_id;
    header.command = cmd;
    header.flag_bits = flag_bits;
    header.packet_number = packet_num;
    header.error_code = errcode;

    if (NULL != extensions)
        memcpy(header.extension_padding, extensions, PROTO_HEADER_EXTENSIONS_SIZE);
}

#define CALC_BODY_LEN(total_len)                ((total_len) - PROTO_HEADER_SIZE)
#define GET_HEADER_ADDR(buf)                    ((void *)buf)
#define GET_BODY_ADDR(buf)                      ((void *)((char *)buf + PROTO_HEADER_SIZE))

enum enum_proto_flag_bit
{
    PROTO_FLAG_BIT_PACKET_END = (1 << 0)
    // TODO: More flags bit definitions in future ...
};

enum enum_packet_end_flag
{
    PACKET_NOT_ENDED = 0,
    PACKET_ENDED = 1
};

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
        return (Int)calns::sys::ntohl64(value);
}

template <typename Int>
static void set_proto_header_field(const int field_offset,
    const Int field_value,
    void *raw_buf)
{
    if (NULL == raw_buf)
        return;

    size_t value_size = sizeof(Int);
    Int *serialized_value_ptr = (Int*) ((char *)raw_buf + field_offset);

    if (sizeof(int8_t) == value_size)
        *serialized_value_ptr = field_value;
    else if (sizeof(int16_t) == value_size)
        *serialized_value_ptr = htons(field_value);
    else if (sizeof(int32_t) == value_size)
        *serialized_value_ptr = htonl(field_value);
    else
        *serialized_value_ptr = calns::sys::htonl64(field_value);
}

int32_t get_proto_length(const void *raw_buf) CA_NOTNULL(1);
inline int32_t get_proto_length(const void *raw_buf)
{
    return get_proto_header_field(raw_buf, HEADER_OFFSET_LENGTH, (int32_t)INVALID_PACKET_LENGTH);
}

void set_proto_length(int32_t value, void *raw_buf) CA_NOTNULL(2);
inline void set_proto_length(int32_t value, void *raw_buf)
{
    set_proto_header_field(HEADER_OFFSET_LENGTH, value, raw_buf);
}

int64_t get_proto_route_id(const void *raw_buf) CA_NOTNULL(1);
inline int64_t get_proto_route_id(const void *raw_buf)
{
    return get_proto_header_field(raw_buf, HEADER_OFFSET_ROUTE_ID, (int64_t)0);
}

void set_proto_route_id(int64_t value, void *raw_buf) CA_NOTNULL(2);
inline void set_proto_route_id(int64_t value, void *raw_buf)
{
    set_proto_header_field(HEADER_OFFSET_ROUTE_ID, value, raw_buf);
}

int32_t get_proto_command(const void *raw_buf) CA_NOTNULL(1);
inline int32_t get_proto_command(const void *raw_buf)
{
    return get_proto_header_field(raw_buf, HEADER_OFFSET_COMMAND, (int32_t)INVALID_COMMAND);
}

void set_proto_command(int32_t value, void *raw_buf) CA_NOTNULL(2);
inline void set_proto_command(int32_t value, void *raw_buf)
{
    set_proto_header_field(HEADER_OFFSET_COMMAND, value, raw_buf);
}

int16_t get_proto_flag_bits(const void *raw_buf) CA_NOTNULL(1);
inline int16_t get_proto_flag_bits(const void *raw_buf)
{
    return get_proto_header_field(raw_buf, HEADER_OFFSET_FLAG_BITS, (int16_t)PROTO_FLAG_BIT_PACKET_END);
}

void set_proto_flag_bits(int16_t value, void *raw_buf) CA_NOTNULL(2);
inline void set_proto_flag_bits(int16_t value, void *raw_buf)
{
    set_proto_header_field(HEADER_OFFSET_FLAG_BITS, value, raw_buf);
}

bool is_final_packet(const void *raw_proto_buf) CA_NOTNULL(1);
inline bool is_final_packet(const void *raw_proto_buf)
{
    int16_t flag_bits = get_proto_flag_bits(raw_proto_buf);

    return (flag_bits & ((int16_t)PROTO_FLAG_BIT_PACKET_END));
}

int16_t get_proto_packet_number(const void *raw_buf) CA_NOTNULL(1);
inline int16_t get_proto_packet_number(const void *raw_buf)
{
    return get_proto_header_field(raw_buf, HEADER_OFFSET_PACKET_NUM, (int16_t)1);
}

void set_proto_packet_number(int16_t value, void *raw_buf) CA_NOTNULL(2);
inline void set_proto_packet_number(int16_t value, void *raw_buf)
{
    set_proto_header_field(HEADER_OFFSET_PACKET_NUM, value, raw_buf);
}

int32_t get_proto_error_code(const void *raw_buf) CA_NOTNULL(1);
inline int32_t get_proto_error_code(const void *raw_buf)
{
    return get_proto_header_field(raw_buf, HEADER_OFFSET_ERR_CODE, (int32_t)0);
}

void set_proto_error_code(int32_t value, void *raw_buf) CA_NOTNULL(2);
inline void set_proto_error_code(int32_t value, void *raw_buf)
{
    set_proto_header_field(HEADER_OFFSET_ERR_CODE, value, raw_buf);
}

//int proto_precheck(const int inlen, const void* inbuf, int &outlen, void* outbuf, int &retcode);

//int validate_protocol_header(void *buf, int len, uint32_t cmd);

int parse_header(const void *inbuf, proto_header_t *result) CA_NOTNULL(1,2);
int assemble_header(const proto_header_t *src, void *outbuf) CA_NOTNULL(1,2);

inline void copy_parsed_header(const proto_header_t &in_header, proto_header_t &out_header)
{
    memcpy(&out_header, &in_header, sizeof(proto_header_t));
}

void copy_serialized_header(const void *inbuf, void *outbuf) CA_NOTNULL(1,2);
inline void copy_serialized_header(const void *inbuf, void *outbuf)
{
    memcpy(outbuf, inbuf, PROTO_HEADER_SIZE);
}

inline bool proto_is_request(const int32_t cmd)
{
    return (0 == cmd % 2);
}

inline bool proto_is_heartbeat(const int32_t cmd)
{
    return (CMD_HEARTBEAT_REQ == cmd ||
        CMD_HEARTBEAT_RESP == cmd);
}

#ifndef USE_JSON_MSG

int get_unified_proto_prefix_length(void);
int get_request_proto_prefix_length(void);
int get_response_proto_prefix_length(void);

#endif // #ifndef USE_JSON_MSG

int extract_session_id(const void *raw_buf, char *result, bool is_req = true);

#ifndef USE_JSON_MSG
inline int parse_message(const void* data, const int size, msg_base &result)
{
    if (!result.ParseFromArray(data, size))
        return -1;

    return result.ByteSize();
}
#else
int parse_message(const void* data, const int size, msg_base &result); // implemented in .cpp file
#endif

inline int get_message_length(const msg_base &msg) // may be time-consuming
{
#ifndef USE_JSON_MSG
    return msg.ByteSize();
#else
    return msg.toStyledString().length();
#endif
}

inline void clear_message_holder(msg_base &holder)
{
#ifndef USE_JSON_MSG
    holder.Clear();
#else
    holder.clear();
#endif
}

inline int serialize_message(const msg_base &msg, void* out_buf)
{
#ifndef USE_JSON_MSG
    int size = msg.ByteSize();

    if (!msg.SerializePartialToArray(out_buf, size))
        return -1;

    return size;
#else
    Json::StreamWriterBuilder writer_factory;
    std::string serialized_str = Json::writeString(writer_factory, msg);

    return serialized_str.copy((char *)out_buf, serialized_str.length());
#endif
}

int serialize_to_buffer(const int32_t out_cmd,
    const msg_base *out_body,
    const void *in_header_buf,
    void *out_buf,
    int &out_len,
    const int32_t errcode = PROTO_RET_SUCCESS,
    const bool is_final_fragment = true,
    const int16_t packet_num = 1);

int send_lite_packet(const int fd,
    const int32_t cmd,
    const int32_t errcode,
    const msg_base *msg_body,
    const int64_t route_id = 0,
    const bool is_final_fragment = true,
    const int16_t packet_num = 1);

// NOTE: This function should be executed by supervisor thread only.
int send_lite_packet(const char *node_type,
    const int max_node_count,
    const int32_t cmd,
    const int32_t errcode,
    const msg_base *msg_body,
    const bool to_all = true,
    const int policy = 0,
    const int64_t route_id = 0,
    const bool is_final_fragment = true,
    const int16_t packet_num = 1);

int send_heartbeat_request(const int fd);
int send_identity_report_request(const int fd);

const char *desc_of_end_flag(int src_value);
/*const char *desc_of_operation_type(int src_value);
const char *desc_of_effect_status(int src_value);*/

} // namespace cafw

extern bool proto_uses_general_prefix(const int32_t cmd);
extern int parse_proto_header_extension(const char *raw_extension, char *parsed_extension) CA_NOTNULL(1,2);
extern int assemble_proto_header_extension(const char *parsed_extension, char *serialized_extension) CA_NOTNULL(1,2);

extern int make_session_id(const void *condition, const int sid_holder_size, char *sid_result);

extern bool session_exists(const char *sid);
extern int fetch_session_info(const char *sid, void* outbuf, int &outlen);
extern int save_session_info(const struct calns::net_connection *src_conn, const void* buf, int buflen, bool commit_now = false);
extern int save_session_info(const struct calns::net_connection *src_conn, const cafw::msg_base *body, const cafw::proto_header_t &header, bool commit_now = false);
extern int update_session_info(const void *buf, int buflen, bool commit_now = false);
extern int update_session_info(const cafw::msg_base *body, const cafw::proto_header_t &header, bool commit_now = false);
extern void clean_expired_sessions(void);

extern bool message_is_time_consuming(const int32_t cmd);

extern bool do_security_check_to_packet(const void* packet);

#endif /* __PROTOCOL_COMMON_H__ */
