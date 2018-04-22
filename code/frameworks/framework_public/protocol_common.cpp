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

#include "protocol_common.h"

#include "base/all.h"
#include "connection_cache.h"
#include "config_manager.h"
#include "resource_manager.h"

namespace cafw
{

/*int proto_precheck(const int inlen, const void* inbuf, int &outlen, void* outbuf, int &retcode)
{
    if (inlen <= 0 ||
        NULL == inbuf ||
        NULL == outbuf)
    {
        retcode = PROTO_RET_UNKNOWN_ERROR;
        GLOG_ERROR_NS("cafw", "null parameter pointers\n");
        return RET_FAILED;
    }

    // TODO

    return RET_OK;
}*/

/*int validate_protocol_header(void *buf, int len, int32_t cmd)
{
    if (NULL == buf)
        return CA_RET(NULL_PARAM);

    if (cmd != get_proto_command(buf))
        return CA_RET(OBJECT_MISMATCHED);

    int32_t expected_total_len = get_proto_length(buf);

    if (len < (int)expected_total_len)
        return CA_RET(LENGTH_TOO_SMALL);

    return RET_OK;
}*/

int parse_header(const void *inbuf, proto_header_t *result)
{
    if (NULL == inbuf || NULL == result)
        return RET_FAILED;

    uint16_t value_int16 = 0;
    uint32_t value_int32 = 0;
    uint64_t value_int64 = 0;

    memcpy(&value_int32, (char *)inbuf + HEADER_OFFSET_LENGTH, sizeof(uint32_t));
    result->length = ntohl(value_int32);

    memcpy(&value_int64, (char *)inbuf + HEADER_OFFSET_ROUTE_ID, sizeof(uint64_t));
    result->route_id = cal::sys::ntohl64(value_int64);

    memcpy(&value_int32, (char *)inbuf + HEADER_OFFSET_COMMAND, sizeof(uint32_t));
    result->command = ntohl(value_int32);

    memcpy(&value_int16, (char *)inbuf + HEADER_OFFSET_FLAG_BITS, sizeof(uint16_t));
    result->flag_bits = ntohs(value_int16);

    memcpy(&value_int16, (char *)inbuf + HEADER_OFFSET_PACKET_NUM, sizeof(uint16_t));
    result->packet_number = ntohs(value_int16);

    memcpy(&value_int32, (char *)inbuf + HEADER_OFFSET_ERR_CODE, sizeof(uint32_t));
    result->error_code = ntohl(value_int32);

    // TODO: how to fill extensions?

    return RET_OK;
}

int assemble_header(const proto_header_t *src, void *outbuf)
{
    if (NULL == src || NULL == outbuf)
        return RET_FAILED;

    uint16_t value_int16 = 0;
    uint32_t value_int32 = 0;
    uint64_t value_int64 = 0;

    value_int32 = htonl(src->length);
    memcpy((char *)outbuf + HEADER_OFFSET_LENGTH, &value_int32, sizeof(uint32_t));

    value_int64 = cal::sys::htonl64(src->route_id);
    memcpy((char *)outbuf + HEADER_OFFSET_ROUTE_ID, &value_int64, sizeof(uint64_t));

    value_int32 = htonl(src->command);
    memcpy((char *)outbuf + HEADER_OFFSET_COMMAND, &value_int32, sizeof(uint32_t));

    value_int16 = htons(src->flag_bits);
    memcpy((char *)outbuf + HEADER_OFFSET_FLAG_BITS, &value_int16, sizeof(uint16_t));

    value_int16 = htons(src->packet_number);
    memcpy((char *)outbuf + HEADER_OFFSET_PACKET_NUM, &value_int16, sizeof(uint16_t));

    value_int32 = htonl(src->error_code);
    memcpy((char *)outbuf + HEADER_OFFSET_ERR_CODE, &value_int32, sizeof(uint32_t));

    // TODO: how to fill extensions?

    return RET_OK;
}

#ifndef USE_JSON_MSG

enum ProtoPrefix
{
    PROTO_PREFIX_UNIFIED = 0,
    PROTO_PREFIX_REQUEST,
    PROTO_PREFIX_RESPONSE
};

static int get_proto_prefix_length_by_type(int type)
{
    int prefix_len = 0;
    const int32_t kInt32Demo = 0xffffffff;
    //const int64_t kInt64Demo = 0xffffffffffffffff;
    std::string kSidDemo(SID_LEN, '1');

    if (PROTO_PREFIX_UNIFIED == type)
    {
        UnifiedBodyPrefix unified_prefix;

        unified_prefix.set_session_id(kSidDemo);

        prefix_len = unified_prefix.ByteSize();
    }
    else if (PROTO_PREFIX_REQUEST == type)
    {
        OldReqBodyPrefix req_prefix;

        req_prefix.set_session_id(kSidDemo);
        req_prefix.set_route_id(kInt32Demo);

        prefix_len = req_prefix.ByteSize();
    }
    else
    {
        OldRespBodyPrefix resp_prefix;

        resp_prefix.set_error_code(kInt32Demo);
        resp_prefix.set_session_id(kSidDemo);
        resp_prefix.set_route_id(kInt32Demo);

        prefix_len = resp_prefix.ByteSize();
    }

    //printf("prefix_len = %d\n", prefix_len);

    return prefix_len;
}

int get_unified_proto_prefix_length(void)
{
    static int s_prefix_len = 0;

    if (s_prefix_len <= 0)
        s_prefix_len = get_proto_prefix_length_by_type(PROTO_PREFIX_UNIFIED);

    return s_prefix_len;
}

int get_request_proto_prefix_length(void)
{
    static int s_prefix_len = 0;

    if (s_prefix_len <= 0)
        s_prefix_len = get_proto_prefix_length_by_type(PROTO_PREFIX_REQUEST);

    return s_prefix_len;
}

int get_response_proto_prefix_length(void)
{
    static int s_prefix_len = 0;

    if (s_prefix_len <= 0)
        s_prefix_len = get_proto_prefix_length_by_type(PROTO_PREFIX_RESPONSE);

    return s_prefix_len;
}

#endif // #ifndef USE_JSON_MSG

int extract_session_id(const void *raw_buf, char *result, bool is_req/* = true*/)
{
    if (NULL == raw_buf || NULL == result)
        return CA_RET(NULL_PARAM);

#ifndef USE_JSON_MSG

    int prefix_len = get_unified_proto_prefix_length();

    if (prefix_len > 0)
    {
        UnifiedBodyPrefix prefix;

        prefix.ParsePartialFromArray(GET_BODY_ADDR(raw_buf), prefix_len);
        memset(result, 0, SID_LEN + 1);
        prefix.session_id().copy(result, SID_LEN);

        return RET_OK;
    }

#else

    Json::Value parsed_msg;
    int total_len = get_proto_length(raw_buf);
    int body_len = CALC_BODY_LEN(total_len);

    if (parse_message(GET_BODY_ADDR(raw_buf), body_len, parsed_msg) > 0)
    {
        if (!parsed_msg[SID_KEY_STR].empty())
        {
            memset(result, 0, SID_LEN + 1);
            parsed_msg[SID_KEY_STR].asString().copy(result, SID_LEN);

            return RET_OK;
        }
    }

#endif // #ifndef USE_JSON_MSG

    return RET_FAILED;
}

#ifdef USE_JSON_MSG
int parse_message(const void* data, const int size, msg_base &result)
{
    char *begin_ptr = (char *)data;
    char *end_ptr = (char *)(begin_ptr + size);
    Json::CharReaderBuilder read_builder;
    JSONCPP_STRING json_error_info;
    Json::CharReader *reader = read_builder.newCharReader();
    bool parse_ok = false;

    if (NULL == reader)
    {
        GLOG_ERROR_NS("cafw", "Json::CharReaderBuilder::newCharReader() failed\n");
        return RET_FAILED;
    }

    parse_ok = reader->parse(begin_ptr, end_ptr, &result, &json_error_info);
    delete reader;
    if (!parse_ok)
    {
        GLOG_ERROR_NS("cafw", "Json::CharReader::parse() failed\n");
        return RET_FAILED;
    }

    return size;
}
#endif

int serialize_to_buffer(const int32_t out_cmd,
    const msg_base *out_body,
    const void *in_header_buf,
    void *out_buf,
    int &out_len,
    const int32_t errcode/* = PROTO_RET_SUCCESS*/,
    const bool is_final_fragment/* = true*/,
    const int16_t packet_num/* = 1*/)
{
    out_len = 0;

    if (NULL == in_header_buf
        || NULL == out_buf)
        return CA_RET(NULL_PARAM);

    int body_len = 0;

    if (NULL != out_body && (body_len = serialize_message(*out_body, GET_BODY_ADDR(out_buf))) < 0)
        return CA_RET(UNDERLYING_ERROR);

    proto_header_t header;

    parse_header(in_header_buf, &header);
    header.length = PROTO_HEADER_SIZE + ((NULL == out_body) ? 0 : body_len);
    header.command = out_cmd;
    if (is_final_fragment)
        header.flag_bits |= PROTO_FLAG_BIT_PACKET_END;
    else
        header.flag_bits &= (~PROTO_FLAG_BIT_PACKET_END);
    header.packet_number = packet_num;
    header.error_code = errcode;
    assemble_header(&header, out_buf);

    out_len = header.length;

    return RET_OK;
}

int send_lite_packet(const int fd,
    const int32_t cmd,
    const int32_t errcode,
    const msg_base *msg_body,
    const int64_t route_id/* = 0*/,
    const bool is_final_fragment/* = true*/,
    const int16_t packet_num/* = 1*/)
{
    if (fd < 0)
        return CA_RET(INVALID_PARAM_VALUE);

    char data[16 * 1024] = {0};
    proto_header_t header;
    int body_len = 0;

    if (NULL != msg_body)
    {
        body_len = serialize_message(*msg_body, GET_BODY_ADDR(data));
        if (body_len < 0)
            return CA_RET(UNDERLYING_ERROR);
    }

    fill_proto_header(header, body_len, cmd, errcode, route_id, packet_num, is_final_fragment);
    assemble_header(&header, data);

    return cal::tcp_base::send_fragment(fd, data, header.length);
}

int send_lite_packet(const char *node_type,
    const int max_node_count,
    const int32_t cmd,
    const int32_t errcode,
    const msg_base *msg_body,
    const bool to_all/* = true */,
    const int policy/* = 0 */,
    const int64_t route_id/* = 0 */,
    const bool is_final_fragment/* = true*/,
    const int16_t packet_num/* = 1*/)
{
    if (NULL == node_type
        || max_node_count <= 0)
        return CA_RET(INVALID_PARAM_VALUE);

    char data[16 * 1024] = {0};
    proto_header_t header;
    int body_len = 0;

    if (NULL != msg_body)
    {
        body_len = serialize_message(*msg_body, GET_BODY_ADDR(data));
        if (body_len < 0)
            return CA_RET(UNDERLYING_ERROR);
    }

    fill_proto_header(header, body_len, cmd, errcode, route_id, packet_num, is_final_fragment);
    assemble_header(&header, data);

    int data_len = header.length;
    const resource_t *res = cal::singleton<resource_manager>::get_instance()->resource();
    connection_cache *conn_caches[] = {
        res->master_connection_cache,
        res->slave_connection_cache
    };
    int ret = -1;

    for (size_t i = 0; i < sizeof(conn_caches) / sizeof(connection_cache*); ++i)
    {
        int bytes_of_this_round = conn_caches[i]->send_to_connections_by_type(node_type, max_node_count, data, data_len, to_all, policy, route_id);
        int tmp_ret = (ret < 0) ? 0 : ret;

        if (to_all)
        {
            ret = (bytes_of_this_round < 0) ? ret : (tmp_ret + bytes_of_this_round);
            continue;
        }

        if (!to_all && bytes_of_this_round < 0)
            continue;

        ret = bytes_of_this_round;
        break; // is not to all, and has sent the packet successfully
    }

    if (ret < 0)
    {
        GLOG_ERROR_NS("cafw", "failed to send message to node with type[%s], ret = %d\n", node_type, ret);
        return RET_FAILED;
    }

    GLOG_DEBUG_NS("cafw", "send done, source single packet length: %d, actual send length: %d\n", data_len, ret);

    return ret;
}

#if defined(HAS_TCP)

int send_heartbeat_request(const int fd)
{
    if (fd < 0)
    {
        GLOG_ERROR_NS("cafw", "fd < 0\n");
        return RET_FAILED;
    }

    return send_lite_packet(fd, CMD_HEARTBEAT_REQ, PROTO_RET_SUCCESS, NULL);
}

int send_identity_report_request(const int fd)
{
    if (fd < 0)
    {
        GLOG_ERROR_NS("cafw", "fd < 0\n");
        return RET_FAILED;
    }

    const int64_t kRouteId = 0;
    char sid[SID_LEN + 1] = {0};
    const net_node_config &self_node = cal::singleton<config_manager>::get_instance()->config_content()->private_configs.self;
    char self_name[cal::MAX_CONNECTION_NAME_LEN + 1] = {0};
    int32_t cmd = CMD_IDENTITY_REPORT_REQ;

    make_session_id(&kRouteId, sizeof(sid), sid);

#ifndef USE_JSON_MSG

    IdentityReportReq msg;

    msg.set_session_id(sid);
    msg.set_server_type(self_node.type_value);
    /*snprintf(self_name, sizeof(self_name), "%s_%s:%u",
        self_node.node_name.c_str(), self_node.node_ip, self_node.node_port);*/
    snprintf(self_name, sizeof(self_name), "%s", self_node.node_name.c_str());
    msg.set_server_name(self_name);
    GLOG_INFO_NS("cafw", "0x%08X | %s | %d | %s\n", cmd, msg.session_id().c_str(), msg.server_type(), msg.server_name().c_str());

#else

    Json::Value msg;

    msg[SID_KEY_STR] = sid;
    msg[SERVER_TYPE_KEY_STR] = self_node.type_value;
    snprintf(self_name, sizeof(self_name), "%s", self_node.node_name.c_str());
    msg[SERVER_NAME_KEY_STR] = self_name;
    GLOG_INFO_NS("cafw", "0x%08X | %s | %d | %s\n", cmd, msg[SID_KEY_STR].asCString(), msg[SERVER_TYPE_KEY_STR].asInt(), msg[SERVER_NAME_KEY_STR].asCString());

#endif // #ifndef USE_JSON_MSG

    return send_lite_packet(fd, cmd, PROTO_RET_SUCCESS, &msg);
}

#endif // #if defined(HAS_TCP)

const char *desc_of_end_flag(int src_value)
{
    const char *desc[] = {
        "not ended",
        "ended"
    };

    if (src_value < 0 || src_value >= (int)(sizeof(desc) / sizeof(char*)))
        return "unknown end flag";

    return desc[src_value];
}

/*const char *desc_of_operation_type(int src_value)
{
    const char *desc[] = {
        "no operation",
        "addition",
        "modification",
        "deletion",
        "cancellation",
        "query"
    };

    if (src_value < 0 || src_value >= (int)(sizeof(desc) / sizeof(char*)))
        return "unknown operation type";

    return desc[src_value];
}

const char *desc_of_effect_status(int src_value)
{
    const char *desc[] = {
        "status disabled",
        "status enabled"
    };

    if (src_value < 0 || src_value >= (int)(sizeof(desc) / sizeof(char*)))
        return "unknown effect status";

    return desc[src_value];
}*/

} // namespace cafw
