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

#include "packet_processor.h"

#include "config_manager.h"
#include "resource_manager.h"
#include "connection_cache.h"
#include "protocol_common.h"
#include "message_cache.h"
#include "handler_component_definitions.h"
#include "customization.h"

extern const char *get_return_code_description(int retcode);

static cafw::handler_component *s_component_tables[] = {
#ifndef IS_DISPATCHER
    g_packet_handler_components,
#endif
    NULL
};

static void clear_components(void)
{
    for (int i = 0; NULL != s_component_tables[i]; ++i)
    {
        cafw::handler_component *component_table = s_component_tables[i];

        for (int j = 0; CMD_UNUSED != component_table[j].in_cmd; ++j)
        {
            cafw::handler_component &component = component_table[j];

            if (NULL != component.partial_in_body)
            {
                delete component.partial_in_body;
                component.partial_in_body = NULL;
            }

            if (NULL != component.whole_in_body)
            {
                delete component.whole_in_body;
                component.whole_in_body = NULL;
            }

            if (NULL != component.out_body)
            {
                delete component.out_body;
                component.out_body = NULL;
            }
        }
    }
}

class component_cleaner
{
public:
    ~component_cleaner()
    {
        clear_components();
    }
};

static component_cleaner s_component_cleaner; // for automatically cleaning g_packet_handler_components

namespace cafw
{

DEFINE_CLASS_NAME(packet_processor);

int packet_processor::m_max_packet_length = PROTO_HEADER_SIZE;

packet_processor::packet_processor()
    : m_message_cache(NULL)
      ,m_component_map(NULL)
      ,m_timestamps_when_pkts_incomplete(NULL)
{
    __inner_init();
}

packet_processor::~packet_processor()
{
    __clear();
}

int packet_processor::build_component_map(void)
{
    if (!is_available())
    {
        GLOG_ERROR_C("processor not initialized yet\n");
        return RET_FAILED;
    }

    m_component_map->clear();

    int ret = RET_OK;

    for (int i = 0; NULL != s_component_tables[i]; ++i)
    {
        for (int j = 0; CMD_UNUSED != s_component_tables[i][j].in_cmd; ++j)
        {
            cafw::handler_component &component = s_component_tables[i][j];

            if (!(m_component_map->insert(std::make_pair(component.in_cmd, component)).second))
                ret = RET_FAILED;
        }
    }

    if (RET_OK != ret)
        m_component_map->clear();

    return ret;
}

#define PACKET_START_FORMAT_LINES(for_heartbeat) if (for_heartbeat) \
    {\
        GLOG_DEBUG("----\n"); \
        GLOG_DEBUG("--------\n"); \
    }\
    else \
    {\
        GLOG_INFO("----\n"); \
        GLOG_INFO("--------\n"); \
    }

#define PACKET_END_FORMAT_LINES(for_heartbeat) if (for_heartbeat) \
    {\
        GLOG_DEBUG("--------\n"); \
        GLOG_DEBUG("----\n"); \
    }\
    else \
    {\
        GLOG_INFO("--------\n"); \
        GLOG_INFO("----\n"); \
    }

int packet_processor::process(const struct cal::net_connection *input_conn,
    int &handled_len,
    struct cal::net_connection **mutable_output_conn,
    int &output_len)
{
    handled_len = 0;
    output_len = 0;

    cal::buffer *in_buf = input_conn->recv_buf;
    void *in_data = in_buf->get_read_pointer();
    int in_fd = input_conn->fd;
    int in_len = in_buf->data_size();

    if (in_fd < 0 ||
        in_len < 0)
    {
        GLOG_ERROR_C("invalid params\n");
        return RET_FAILED;
    }

    if (0 == in_len)
    {
        GLOG_INFO("no need to handle empty packet\n");
        return RET_OK;
    }

    int32_t length = get_proto_length(in_data);
    int32_t command = get_proto_command(in_data);
    const int kMaxErrorSeconds = 5; // TODO: How about take it from the configuration file?
    const int64_t kCurUtcSecs = cal::time_util::get_utc_seconds();
    const char *kConnName = input_conn->peer_name;

    if (in_len < length || in_len < (int)PROTO_HEADER_SIZE)
    {
        GLOG_WARN_C("incomplete packet in recv_buf of connection[fd:%d, name:%s]: expected length = %d, actual length = %d,"
            " minimum length = %d, command code = 0x%08X, fd = %d, may need more bytes and handle them later\n",
            in_fd, kConnName, length, in_len,
            (int)PROTO_HEADER_SIZE, command, in_fd);

        std::map<std::string, int64_t>::iterator time_iter = m_timestamps_when_pkts_incomplete->find(kConnName);

        // Initialization, DO NOT miss it!
        if (m_timestamps_when_pkts_incomplete->end() == time_iter)
            time_iter = m_timestamps_when_pkts_incomplete->insert(std::map<std::string, int64_t>::value_type(kConnName, 0)).first;

        int64_t *time_ptr = &(time_iter->second);

        // records the start time of error
        if (0 == *time_ptr)
            *time_ptr = kCurUtcSecs;

        bool packet_too_big = (length > in_buf->total_size());
        bool is_timed_out = (kCurUtcSecs - *time_ptr >= kMaxErrorSeconds);

        if (is_timed_out || packet_too_big)
        {
            GLOG_WARN_C("too many retries or packet too big, recv_buf of connection[fd:%d, name:%s, size: %d]"
                " may contains bad data, reset it now\n", in_fd, kConnName, in_buf->total_size());
            in_buf->reset();
            *time_ptr = 0; // DO NOT forget clearing it after error was handled!
        }

        if (is_timed_out)
            return CA_RET(OPERATION_TIMED_OUT);
        else if (packet_too_big)
            return CA_RET(LENGTH_TOO_BIG);
        else
            return CA_RET(RESOURCE_NOT_AVAILABLE);
    }

    handled_len = (length > 0) ? length : in_len;

    if (length <= 0)
    {
        GLOG_ERROR_C("invalid packet_len[%d], ignore it, and skip %d bytes directly\n",
            length, in_len);
        return RET_FAILED;
    }

    int16_t packet_num = get_proto_packet_number(in_data);
    int16_t flag_bits = get_proto_flag_bits(in_data);
    int64_t route_id = get_proto_route_id(in_data);
    int32_t error_code = get_proto_error_code(in_data);
    bool is_heartbeat = proto_is_heartbeat(command);
    bool is_identity_report = (CMD_IDENTITY_REPORT_REQ == command || CMD_IDENTITY_REPORT_RESP == command);

    PACKET_START_FORMAT_LINES(is_heartbeat);

    if (!is_heartbeat)
    {
        GLOG_INFO("%d bytes new packet from connection{ fd[%d] | name[%s] | address[%s:%hu] }:"
            " header{ length[%d] | route_id[%ld] | command[0x%08X] | flag_bits[0x%04X]"
            " | packet_number[%hd] | error_code[%d] }\n",
            in_len, in_fd, input_conn->peer_name, input_conn->peer_ip, input_conn->peer_port,
            length, route_id, command, flag_bits,
            packet_num, error_code);
    }

#ifdef VALIDATES_CONNECTION
    if (!input_conn->is_validated && !is_identity_report)
    {
        if (!is_heartbeat)
        {
            GLOG_ERROR_C("this connection has not been validated yet,"
                " any packets except [0x%08X] or [0x%08X] will be ignored\n",
                CMD_IDENTITY_REPORT_REQ, CMD_IDENTITY_REPORT_RESP);
            PACKET_END_FORMAT_LINES(false);
        }
        return RET_FAILED;
    }
#endif

    int ret = -1;

    if (is_heartbeat || is_identity_report)
    {
        ret = diagnose_connection(input_conn, length, mutable_output_conn, output_len);
        PACKET_END_FORMAT_LINES(is_heartbeat);
        return ret;
    }

#ifdef IS_DISPATCHER
    ret = dispacher_general_flow(input_conn, length, mutable_output_conn, output_len);
    PACKET_END_FORMAT_LINES(false);
#else
    bool is_req = proto_is_request(command);
    char sid[SID_LEN + 1] = {0};
    void* out_data_ptr = (*mutable_output_conn)->send_buf->get_write_pointer();

    component_map::iterator it = m_component_map->find(command);
    if (m_component_map->end() == it)
    {
        GLOG_ERROR_C("unknown command code 0x%08X, packet discarded, fd = %d\n", command, in_fd);
        ret = RET_FAILED;
        goto RETURN;
    }

    if (it->second.filters_repeated_session)
        extract_session_id(in_data, sid, is_req);

    if (it->second.filters_repeated_session && session_exists(sid))
    {
        if (is_req)
        {
            GLOG_INFO("session[%s] had been handled and saved in database,"
                " use this info for fast reply\n", sid);
            ret = fetch_session_info(sid, out_data_ptr, output_len);
        }
        else
        {
            GLOG_INFO("there's no need to handle response of session[%s], unless it is"
                " part of a multi-step process which has to be carefully dealt with\n", sid);
            ret = RET_OK;
        }
        goto RETURN;
    }

    ret = single_operator_general_flow(input_conn, length, it->second, mutable_output_conn, output_len);

    if (it->second.filters_repeated_session
        && output_len > 0)
    {
        // Has to fetch the pointer again, in case that the output connection changes
        // within SingleOperatorGeneralFlow().
        out_data_ptr = (*mutable_output_conn)->send_buf->get_write_pointer();

        save_session_info(input_conn, out_data_ptr, output_len, true);
    }

RETURN:

    PACKET_END_FORMAT_LINES(false);
#endif
    return ret;
}

int packet_processor::single_operator_general_flow(const struct cal::net_connection *input_conn,
    const int input_len,
    handler_component &component,
    struct cal::net_connection **mutable_output_conn,
    int &output_len)
{
    output_len = 0;

    int32_t command = component.in_cmd;
    int retcode = PROTO_RET_SUCCESS;
    msg_base *partial_in_body = component.partial_in_body;
    int partial_in_body_len = 0;
    msg_base *whole_in_body = component.whole_in_body;
    int whole_in_body_len = 0;
    msg_base *actual_body_for_parsing = component.has_multi_fragments ? partial_in_body : whole_in_body;
    int &body_len_after_parsing = component.has_multi_fragments ? partial_in_body_len : whole_in_body_len;
    msg_cache_value *msg_cache_item = NULL;
    msg_base *out_body = component.out_body;
    char body_container_type[128] = {0};
    void* in_data_ptr = input_conn->recv_buf->get_read_pointer();
    int32_t total_len = get_proto_length(in_data_ptr);
    int32_t body_len = CALC_BODY_LEN(total_len);
    const char *sid = "see_sid_in_other_places";
    const int sid_len = SID_LEN;
    int16_t packet_num = 1;
    int64_t start_time = cal::time_util::get_utc_microseconds();
    int64_t last_step_time = start_time;
    int64_t cur_step_time = start_time;
    bool all_parsed_ok = false;
#ifndef USE_JSON_MSG
    bool prefix_parsed_ok = false;
    // DO NOT use such a pointer to operate *_in_body, it will corrupt the contents, use an object instead!
    //MinimalBody *body_prefix = NULL;
    MinimalBody body_prefix;
#endif
    bool has_done_business = false;

#define STAT_TIME_CONSUMPTION(op_literal) cur_step_time = cal::time_util::get_utc_microseconds(); \
    GLOG_INFO("[cmd:0x%08X] ["op_literal"] done, time spent: %ld us\n", command, cur_step_time - last_step_time); \
    last_step_time = cur_step_time

    if (NULL != out_body)
        clear_message_holder(*out_body);

    /*
     * Parses data from buffer.
     */

    clear_message_holder(*actual_body_for_parsing);
    body_len_after_parsing = parse_message(GET_BODY_ADDR(in_data_ptr), body_len, *actual_body_for_parsing);
    all_parsed_ok = (body_len_after_parsing > 0);
    //body_base = dynamic_cast<UnifiedBodyBase *>(actual_body_for_parsing); // failed
    //body_base = reinterpret_cast<UnifiedBodyBase *>(actual_body_for_parsing); // static_cast works too
    STAT_TIME_CONSUMPTION("input packet parsing");

    if (!all_parsed_ok)
    {
#ifndef USE_JSON_MSG
        GLOG_ERROR_C("protocol parse function failed\n");
#else
        char *failed_msg = (char *)calloc(body_len + 1, sizeof(char));

        if (NULL != failed_msg)
        {
            memcpy(failed_msg, GET_BODY_ADDR(in_data_ptr), body_len);
            GLOG_ERROR_C("protocol parse function failed, message parsed unsuccessfully: %s\n",
                (NULL != failed_msg) ? failed_msg : "");
            free(failed_msg);
        }
        else
            GLOG_ERROR_C("protocol parse function failed\n");
#endif // #ifndef USE_JSON_MSG

        return RET_FAILED;
    }

    if (!proto_uses_general_prefix(command))
    {
        strncpy(body_container_type, typeid(*whole_in_body).name(), sizeof(body_container_type));
        component.do_business(input_conn, whole_in_body, mutable_output_conn, out_body, retcode);
        has_done_business = true;
        STAT_TIME_CONSUMPTION("business operation");
        goto OUTPUT;
    }

#ifndef USE_JSON_MSG
    // Why do we has to do a partial parse again? Because msg_base is a base class and has no member named session_id.
    prefix_parsed_ok = body_prefix.ParseFromArray(GET_BODY_ADDR(in_data_ptr), get_unified_proto_prefix_length());
    //GLOG_DEBUG_C("prefix_parsed_ok = %d\n", prefix_parsed_ok);
    sid = body_prefix.session_id().c_str();
#else
    if (!(*actual_body_for_parsing)[SID_KEY_STR].empty())
        sid = (*actual_body_for_parsing)[SID_KEY_STR].asCString();
#endif
    packet_num = get_proto_packet_number(in_data_ptr);

    /*
     * Step 1: Group fragments together, for multi-packet case only.
     */

    if (component.has_multi_fragments)
    {
        /*GLOG_DEBUG("has_multi_fragments = %d, whole_in_body will be assigned with value found"
            " from cache\n",handler.has_multi_fragments);*/
        if (NULL == (msg_cache_item = (msg_cache_value*)m_message_cache->find(sid, sid_len)))
        {
            if (1 != packet_num)
            {
                GLOG_ERROR_C("message[%s] not found, packet num = %d\n", sid, packet_num);
                m_message_cache->print();
                return RET_FAILED;
            }

            whole_in_body = component.alloc_body_container();
            msg_cache_item = char_dict_alloc_val< msg_cache_value >();
            msg_cache_item->from_fd = input_conn->fd;
            //msg_cache_item->from_name = in.connection->peer_name;
            msg_cache_item->cmd = command;
            msg_cache_item->contents = whole_in_body;
            if (RET_FAILED == m_message_cache->add(sid, sid_len, msg_cache_item, sizeof(msg_cache_item)))
            {
                GLOG_ERROR_C("failed to add message[%s] into cache, packet num = %d\n",
                    sid, packet_num);
                m_message_cache->print();
                delete whole_in_body;
                char_dict_release_val< msg_cache_value >(&msg_cache_item);
                return RET_FAILED;
            }
            GLOG_INFO("message[%s] added into cache, packet num = %d, address of message = %p, "
                "address of cache VALUE = %p\n", sid, packet_num, whole_in_body, &whole_in_body);
        }
        GLOG_INFO("message[%s] found from cache, packet num = %d, address of message = %p, "
            "address of cache VALUE = %p\n", sid, packet_num, msg_cache_item, &msg_cache_item);

        whole_in_body = (msg_base *)(msg_cache_item->contents);

        if (RET_FAILED == component.group_fragments(partial_in_body, whole_in_body))
        {
            GLOG_ERROR_C("failed to group fragment, SID = %s, packet num = %d\n",
                sid, packet_num);
            return RET_FAILED;
        }
        GLOG_INFO("fragment[%d] grouped into cached packet\n", packet_num);

        msg_cache_item->last_op_time = cal::time_util::get_utc_microseconds();
        STAT_TIME_CONSUMPTION("fragment grouping");

        bool is_end = is_final_packet(in_data_ptr);

        if (is_end)
        {
            GLOG_INFO("part[%d] of packet(%s) received, expected total len = %d, actual buffer len = %d, "
                "bodylen = %d, and END flag found, starting detail process ...\n",
                packet_num, sid, total_len, input_len, partial_in_body_len);
        }
        else
        {
            GLOG_INFO("part[%d] of packet(%s) received, expected total len = %d, actual buffer len = %d, "
                "bodylen = %d, waiting for more parts to complete the process ...\n",
                packet_num, sid, total_len, input_len, partial_in_body_len);
            return RET_OK;
        }
    } // end if (component.has_multi_fragments)

    strncpy(body_container_type, typeid(*whole_in_body).name(), sizeof(body_container_type));
    GLOG_INFO("%s was parsed successfully, cmd = 0x%08X, desc = %s,"
        " sid = %s, total bodylen = %d, in_fd = %d\n",
        body_container_type, command, component.description,
        sid, get_message_length(*whole_in_body), input_conn->fd);

    /*
     * Step 2: Do validation if required.
     */

    if (NULL != component.in_packet_is_ok && !component.in_packet_is_ok(NULL, whole_in_body, retcode))
    {
        GLOG_ERROR_C("input packet validation failed\n");
        goto OUTPUT;
    }
    STAT_TIME_CONSUMPTION("input packet validation");

    /*
     * Step 3: Execute business operation. This is an required step for all business types.
     */

    has_done_business = true;
    if (RET_OK != component.do_business(input_conn, whole_in_body, mutable_output_conn, out_body, retcode))
    {
        GLOG_ERROR_C("core business operation failed\n");
        goto OUTPUT;
    }
    STAT_TIME_CONSUMPTION("business operation");

OUTPUT:

    do {
        if (!has_done_business)
            break;

        if (PROTO_RET_SUCCESS == retcode)
        {
            if (NULL == component.commit_database_modification)
                GLOG_INFO("business finished successfully without database modifications,"
                    " or with database committing inside the business handler.\n");
            else
            {
                component.commit_database_modification(-1);
                GLOG_INFO("business finished successfully, commit database modifications now\n");
            }
        }
        else
        {
            if (NULL == component.rollback_database_modification)
                GLOG_WARN("business is not ok, but no database roll-back is needed,"
                    " or the roll-back is inside the business handler.\n");
            else
            {
                component.rollback_database_modification(-1);
                GLOG_WARN("business is not ok, roll back database modifications now\n");
            }
        }
        STAT_TIME_CONSUMPTION("database commit or roll-back");
    } while (0);

    /*
     * Step 4: Assemble output packet if required.
     */
    if (NULL != out_body && NULL != component.assemble_out_packet)
    {
        component.assemble_out_packet(in_data_ptr, retcode, whole_in_body, out_body);
        STAT_TIME_CONSUMPTION("output packet assembling");
    }

    /*
     * Step 5: Serialize output data(if existed) to send buffer.
     */
    if (NULL != out_body && get_message_length(*out_body) > 0)
    {
        cal::net_connection *output_conn = *(mutable_output_conn);
        cal::buffer *out_buf = output_conn->send_buf;
        void* out_data_ptr = out_buf->get_write_pointer();
        int available_buf_len = out_buf->total_size() - out_buf->write_position();
        bool no_enough_space = ( available_buf_len < 2 * get_current_max_packet_length()
            || available_buf_len > out_buf->total_size() );

        if (no_enough_space)
        {
            GLOG_WARN("send_buf of connection[%d|%s] has little space left, trying to adjust it ...\n",
                output_conn->fd, output_conn->peer_name);
            cal::tcp_base::send_from_connection(output_conn);
        }

        available_buf_len = out_buf->total_size() - out_buf->write_position(); // refresh again, so does statements below
        out_data_ptr = out_buf->get_write_pointer();

        no_enough_space = ( available_buf_len < 2 * get_current_max_packet_length()
            || available_buf_len > out_buf->total_size() );
        if (no_enough_space)
        {
            GLOG_ERROR("still can not get enough space to hold the output, discard it\n");
            return RET_FAILED;
        }

        if (serialize_to_buffer(component.out_cmd, out_body, in_data_ptr, out_data_ptr, output_len, retcode) < 0)
        {
            GLOG_ERROR_C("serialize_to_buffer() failed\n");
            return RET_FAILED;
        }
        STAT_TIME_CONSUMPTION("output data serialization");
        update_max_packet_length(output_len);

        GLOG_INFO("%d bytes output packet generated and loaded into send buffer"
            " of connection{ fd[%d] | name[%s] | address[%s:%u] }\n",
            output_len, output_conn->fd, output_conn->peer_name, output_conn->peer_ip, output_conn->peer_port);
    }

#if 1 // disable this operation to clean up messages in another place and increase the possibility of hash collision for test
    // delete message from cache. do it after all operations to avoid unexpectedly freeing memories needed by others
    if (component.has_multi_fragments)
        m_message_cache->del(sid, sid_len);
#endif
    if (PROTO_RET_SUCCESS == retcode)
        GLOG_INFO("~ ~ ~ ~ ~ ~ ~ ~ ~ ~ %s::%s() for [ 0x%08X | %s | %s ] successful, total time spent: %ld us\n",
            class_name(), __FUNC__, command, component.description, sid, cal::time_util::get_utc_microseconds() - start_time);
    else
        GLOG_ERROR("! ! ! ! ! ! ! ! ! ! %s::%s() for [ 0x%08X | %s | %s ] failed,"
            " ret = %d, desc = %s, total time spent: %ld us\n",
            class_name(), __FUNC__, command, component.description, sid,
            retcode, get_return_code_description(retcode), cal::time_util::get_utc_microseconds() - start_time);

    return RET_OK;
}

int packet_processor::dispacher_general_flow(const struct cal::net_connection *input_conn,
    const int input_len,
    struct cal::net_connection **mutable_output_conn,
    int &output_len)
{
#if 0 // TODO
    cal::net_connection *connection = in.connection;
    void *in_data = in.data_ptr;
    int in_len = get_proto_length(in_data);
    int32_t command = get_proto_command(in_data);
    int retcode = PROTO_RET_SUCCESS;

    const char *sid = "NULL";
    int sid_len = 0;
    int route_id = -1;
    bool prefix_parsed_ok = false;
    bool is_req = proto_is_request(command);
#ifdef USE_UNIFIED_PROTO_PREFIX
    UnifiedBodyBase body_base;
    UnifiedBodyPrefix *prefix = NULL;
    int unified_prefix_len = get_unified_proto_prefix_length();
#else
    RequestBodyIndex req_index;
    ResponseBodyIndex resp_index;
    int unified_prefix_len = is_req
        ? get_request_proto_index_length()
        : get_response_proto_index_length();
#endif
    int parsable_len_for_prefix = (unified_prefix_len <= in_len) ? unified_prefix_len : in_len;

#ifdef USE_UNIFIED_PROTO_PREFIX
    prefix_parsed_ok = body_base.ParseFromArray(GET_BODY_ADDR(in_data), parsable_len_for_prefix);

    prefix = body_base.mutable_bodyprefix();
    sid = prefix->sid().c_str();
    sid_len = prefix->sid().length();
    retcode = prefix->has_retcode() ? prefix->retcode() : 0;
    route_id = prefix->route_id();
#else
    if (is_req)
    {
        prefix_parsed_ok = req_index.ParseFromArray(GET_BODY_ADDR(in_data), parsable_len_for_prefix);
        sid = req_index.sid().c_str();
        sid_len = req_index.sid().length();
        retcode = 0;
        route_id = req_index.route_id();
    }
    else
    {
        prefix_parsed_ok = resp_index.ParseFromArray(GET_BODY_ADDR(in_data), parsable_len_for_prefix);
        sid = resp_index.sid().c_str();
        sid_len = resp_index.sid().length();
        retcode = resp_index.retcode();
        route_id = resp_index.route_id();
    }
#endif

    // TODO: It hardly succeeds. Weird!
    /*if (!prefix_parsed_ok)
    {
        LOG_ERROR_CV("protobuf::ParseFromArray() failed, anyway, try to glimpse session_id: %s\n", sid);
        return RET_FAILED;
    }*/

    LOG_INFO("protobuf::ParseFromArray() done, it's a [%s], parsable_len_for_prefix = %d,"
        " prefix_parsed_ok = %s, session_id = %s, ID length = %d, route_id = %d, retcode exists and is %d if it's a RESPONSE\n",
        is_req ? "REQUEST" : "RESPONSE", parsable_len_for_prefix,
        prefix_parsed_ok ? "TRUE" : "FALSE", sid, sid_len, route_id, retcode);

    bool is_resp = (!is_req);
    msgval *_msgval = (msgval*)m_message_cache->return_as_index(sid, sid_len);

    if (is_resp && NULL == _msgval) // (1) a response missing source info
    {
        LOG_ERROR_CV("can not find source info from message cache, session_id = %s\n", sid);
        return RET_FAILED;
    }

    int64_t cur_time = cal::TimeHelper::GetUtcMicroseconds();
    const int kSendTryCount = 3;
    static const char *s_operator_type = cal::singleton<ConfigManager>::instance()->config_entries()->private_configs.self.type_name.c_str();
    static int s_dispatch_policy = CFG_GET_DISPATCH_POLICY();
    int send_ret = -1;
    const resource_t *res = cal::singleton<resource_manager>::instance()->resource_entries();
    connection_cache *master_conn_cache = res->master_connection_cache;
    connection_cache *slave_conn_cache = res->slave_connection_cache;

    if (is_req && NULL == _msgval) // (2) a new request to be forwarded
    {
        if (NULL == (_msgval = char_dict_alloc_val< msgval >()))
        {
            LOG_ERROR_CV("failed to allocate memory for message cache\n");
            return RET_FAILED;
        }

        dict_entry_ptr src_owner = (dict_entry_ptr)(connection->owner);

        _msgval->reqcmd = command;
        _msgval->from_fd = in.fd;
        _msgval->from_name = (NULL != src_owner) ? GET_CHAR_DICT_KEY(src_owner) : NULL;
        _msgval->to_fd = -1;
        _msgval->to_name = NULL;
        _msgval->msg = NULL;
        _msgval->last_op_time = cur_time;

        if (m_message_cache->add(sid, sid_len, _msgval, sizeof(msgval)) < 0)
        {
            LOG_ERROR_CV("failed to add item into message cache\n");
            return RET_FAILED;
        }

        for (int i = 0; i < kSendTryCount; ++i)
        {
            net_conn_index *conn_index = master_conn_cache->PickOneConnection(s_operator_type, s_dispatch_policy, route_id);

            if (NULL == conn_index &&
                NULL == (conn_index = slave_conn_cache->PickOneConnection(s_operator_type, s_dispatch_policy, route_id)))
            {
                LOG_ERROR_CV("all servers of type[%s] are dead,"
                    " forward operation aborts.\n", s_operator_type);
                return RET_FAILED;
            }

            cal::net_connection *conn_detail = conn_index->conn_detail;

            if ((send_ret = cal::tcp_base::SendFragment(conn_detail->fd, in_data, in_len)) < 0)
            {
                LOG_WARN_CV("failed to forward packet to server{ fd[%d], name[%s], address[%s:%u] },"
                    " re-send it, or pick another server to try again\n",
                    conn_detail->fd, conn_detail->peer_name, conn_detail->peer_ip, conn_detail->peer_port);
                continue;
            }
            LOG_INFO("packet forwarded successfully to server{ fd[%d], name[%s], address[%s:%u] },"
                " expected length = %d, actual length = %d\n",
                conn_detail->fd, conn_detail->peer_name, conn_detail->peer_ip, conn_detail->peer_port,
                in_len, send_ret);

            dict_entry_ptr dst_owner = (dict_entry_ptr)(conn_detail->owner);

            _msgval->to_fd = conn_detail->fd;
            _msgval->to_name = (NULL != dst_owner) ? GET_CHAR_DICT_KEY(dst_owner) : NULL;

            LOG_INFO("route info added into message cache: { from_fd[%d] | from_name[%s] }"
                " ----> { to_fd[%d] | to_name[%s] }\n",
                _msgval->from_fd, (NULL != _msgval->from_name) ? _msgval->from_name : "/",
                _msgval->to_fd, (NULL != _msgval->to_name) ? _msgval->to_name : "/");

            return RET_OK;
        }

        LOG_ERROR_CV("failed to forward packet\n");
        return RET_FAILED;
    }

    /*
     * (3) a fragment of a multi-packet request
     * (4) a normal response
     */

    LOG_INFO("source info found successfully from message cache with session_id\n");

    _msgval->last_op_time = cur_time;

    const char *target_name = is_req ? _msgval->to_name : _msgval->from_name;
    int target_fd = is_req ? _msgval->to_fd : _msgval->from_fd;
    dict_entry_ptr owner = NULL;

    if (NULL == target_name)
    {
        LOG_WARN_CV("%s in message cache is not available,"
            " current packet has to be forwarded with %s, fd = %d\n",
            (is_req ? "to_name" : "from_name"), (is_req ? "to_fd" : "from_fd"), target_fd);
    }
    else
    {
        int name_len = strlen(target_name);

        owner = (dict_entry_ptr)master_conn_cache->return_as_entry(target_name, name_len);
        if (NULL == owner &&
            NULL == (owner = (dict_entry_ptr)slave_conn_cache->return_as_entry(target_name, name_len)))
        {
            LOG_WARN_CV("can not find item from connection cache with name[%s],"
                " current packet has to be forwarded with %s, fd = %d\n",
                target_name, (is_req ? "to_fd" : "from_fd"), target_fd);
        }
        else
        {
            LOG_INFO("found item from connection cache with name[%s], it's the target"
                " which the packet should be forwarded to\n", target_name);

            net_conn_index *target_conn_index = (net_conn_index*)GET_CHAR_DICT_VALUE(owner);

            if (target_fd != target_conn_index->fd)
            {
                LOG_WARN_CV("fd updated from %d to %d\n", target_fd, target_conn_index->fd);
                target_fd = target_conn_index->fd;
            }
        }
    }

    send_ret = cal::tcp_base::SendFragment(target_fd, in_data, in_len);
    if (send_ret <= 0)
    {
        LOG_ERROR_CV("failed to forward packet to fd[%d], ret = %d\n", target_fd, send_ret);
        return RET_FAILED;
    }

    LOG_INFO("forward operation successful, expected length = %d, actual length = %d\n",
        in_len, send_ret);

    return RET_OK;
#endif
    return RET_FAILED;
}

static int general_heartbeat_handling(
    const int fd,
    const char *heartbeat_type,
    const msg_base *in_body,
    msg_base *out_body)
{
    cal::net_connection *connection = NULL;
    const char *peer_name = NULL;
    bool is_req = (0 == strcmp(heartbeat_type, "request"));

    if (is_req)
    {
        cal::tcp_server *listener = cal::singleton<resource_manager>::get_instance()->resource()->server_listener;

        connection = listener->find_peer(fd);
        peer_name = "unknown_client";
    }
    else
    {
        cal::tcp_client *requester = cal::singleton<resource_manager>::get_instance()->resource()->client_requester;

        connection = requester->find_peer(fd);
        peer_name = "unknown_server";
    }


    if (NULL != connection)
    {
        connection->last_op_time = cal::time_util::get_utc_microseconds();
        peer_name = connection->peer_name;
    }

    GLOG_DEBUG("^~^~^~^~ heart beat %s: fd[%d], name[%s]\n",
        heartbeat_type, fd, peer_name);
#if 0 // TODO
    if (NULL != out_body)
        ((OldHeartbeatResp *)out_body)->Clear();

    if (is_req)
        ((OldHeartbeatResp *)out_body)->set_session_id(((OldHeartbeatReq *)in_body)->session_id().c_str());
#endif
    return RET_OK;
}

DECLARE_BUSINESS_FUNC(heartbeat_request_handling)
{
    return general_heartbeat_handling(in_conn->fd, "request", in_body, out_body);
}

DECLARE_BUSINESS_FUNC(heartbeat_response_handling)
{
    return general_heartbeat_handling(in_conn->fd, "response", in_body, out_body);
}

DECLARE_BUSINESS_FUNC(identity_report_request_handling)
{
    cal::net_connection *connection = NULL;
    int fd = in_conn->fd;
#ifndef USE_JSON_MSG
    IdentityReportReq *req = (IdentityReportReq *)in_body;
    IdentityReportResp *resp = (IdentityReportResp *)out_body;
    const char *sid = req->session_id().c_str();
    int server_type = req->server_type();
    const char *client_name = req->server_name().c_str();
#else
    Json::Value *req = (Json::Value *)in_body;
    Json::Value *resp = (Json::Value *)out_body;
    const Json::Value &sid_json_value = (*req)[SID_KEY_STR];
    const char *sid = sid_json_value.empty() ? "" : sid_json_value.asCString();
    const Json::Value &server_type_json_value = (*req)[SERVER_TYPE_KEY_STR];
    int server_type = server_type_json_value.empty() ? 0 : server_type_json_value.asInt();
    const Json::Value &client_name_json_value = (*req)[SERVER_NAME_KEY_STR];
    const char *client_name = client_name_json_value.empty() ? "" : client_name_json_value.asCString();
#endif
    cal::tcp_server *listener = cal::singleton<resource_manager>::get_instance()->resource()->server_listener;

    GLOG_INFO("identity report request contents: session_id[%s] | server_type[%d] | server_name[%s]\n", sid, server_type, client_name);

    retcode = PROTO_RET_SUCCESS;

    connection = listener->find_peer(fd);
    if (NULL == connection)
    {
        GLOG_WARN_NS("cafw", "peer node with fd[%d] is not a client, ignore this request\n", fd);
        return RET_FAILED;
    }

#ifndef USE_JSON_MSG
    resp->set_session_id(sid);
#else
    (*resp)[SID_KEY_STR] = sid;
#endif

    int name_len = strlen(client_name);
    connection_cache *conn_cache = cal::singleton<resource_manager>::get_instance()->resource()->master_connection_cache;
    net_conn_index *conn_index = conn_cache->return_as_index(client_name, name_len);
    const config_content_t *conf_contents = cal::singleton<config_manager>::get_instance()->config_content();
    const std::map<std::string, int> &conf_server_types = conf_contents->fixed_common_configs.server_types;

    snprintf(connection->peer_name, sizeof(connection->peer_name), "%s", client_name);
    GLOG_INFO("client name set to %s\n", connection->peer_name);

    if (NULL == conn_index)
    {
        GLOG_INFO("this request is from a new client, adding the client into connection cache ...\n");

        conn_index = char_dict_alloc_val< net_conn_index >();
        if (NULL == conn_index)
        {
            GLOG_ERROR_NS("cafw", "char_dict_alloc_val() for %s failed\n", client_name);
            return RET_FAILED;
        }

        memset(conn_index->server_type, 0, sizeof(conn_index->server_type));
        //strncpy(conn_index->server_type, "client", sizeof(conn_index->server_type) - 1);
        for (std::map<std::string, int>::const_iterator iter = conf_server_types.begin(); iter != conf_server_types.end(); ++iter)
        {
            if (server_type == iter->second)
            {
                iter->first.copy(conn_index->server_type, iter->first.length());
                break;
            }
        }
        conn_index->is_server = false;
        memset(conn_index->conn_alias, 0, sizeof(conn_index->conn_alias));
        strncpy(conn_index->conn_alias, client_name, sizeof(conn_index->conn_alias) - 1);
        memset(conn_index->peer_ip, 0, sizeof(conn_index->peer_ip));
        strncpy(conn_index->peer_ip, connection->peer_ip, cal::IPV4_LEN);
        conn_index->peer_port = connection->peer_port;
        conn_index->fd = fd;
        conn_index->conn_detail = connection;
        conn_index->attribute.is_master = true;

        if (conn_cache->add(client_name, name_len, conn_index, sizeof(net_conn_index)) < 0)
        {
            GLOG_ERROR_NS("cafw", "ConnectionCache::Add() for %s failed\n", client_name);
            char_dict_release_val(&conn_index);
            return RET_FAILED;
        }

        GLOG_INFO("client{ conn_alias[%s] | fd[%d] | address[%s:%u]"
            " | server_type[%s] | is_server[%d] } added successfully\n",
            conn_index->conn_alias, conn_index->fd, conn_index->peer_ip, conn_index->peer_port,
            conn_index->server_type, conn_index->is_server);
    }
    else
    {
        if (NULL != conn_index->conn_detail)
        {
            GLOG_ERROR_NS("cafw", "found an active client with the same name[%s] in connection cache,"
                " current client is ignored\n", client_name);
            return RET_FAILED;
        }

        GLOG_INFO("this request is from an old client which disconnected and reconnected again,"
            " updating the its info in connection cache ...\n");

        memset(conn_index->peer_ip, 0, sizeof(conn_index->peer_ip));
        strncpy(conn_index->peer_ip, connection->peer_ip, cal::IPV4_LEN);
        conn_index->peer_port = connection->peer_port;
        conn_index->fd = fd;
        conn_index->conn_detail = connection;

        GLOG_INFO("connection cache info of client[%s] partially updated: fd[%d],"
            " address[%s:%u], conn_detail[%p]\n", client_name, conn_index->fd,
            conn_index->peer_ip, conn_index->peer_port, conn_index->conn_detail);
    }

    dict_entry_ptr owner_ptr = conn_cache->return_as_entry(client_name, name_len);

    if (NULL == owner_ptr)
    {
        GLOG_ERROR_NS("cafw", "can not find iterator of connection cache item with name[%s]\n", client_name);
        return RET_FAILED;
    }
    connection->owner = owner_ptr;
    GLOG_INFO("finished updating owner info for this client\n");

    connection->is_validated = true;
    GLOG_INFO("validation status set to %d, will return %d\n", connection->is_validated, retcode);

    return RET_OK;
}

DECLARE_BUSINESS_FUNC(identity_report_response_handling)
{
    cal::net_connection *connection = NULL;
    int fd = in_conn->fd;
#ifndef USE_JSON_MSG
    IdentityReportResp *resp = (IdentityReportResp *)in_body;
    const char *sid = resp->session_id().c_str();
#else
    Json::Value *resp = (Json::Value *)in_body;
    const Json::Value &sid_json_value = (*resp)[SID_KEY_STR];
    const char *sid = sid_json_value.empty() ? "" : sid_json_value.asCString();
#endif
    cal::tcp_client *requester = cal::singleton<resource_manager>::get_instance()->resource()->client_requester;

    GLOG_INFO("identity report response contents: error_code[%d] | session_id[%s]\n", retcode, sid);

    connection = requester->find_peer(fd);
    if (NULL == connection)
    {
        GLOG_WARN_NS("cafw", "peer node with fd[%d] is not a server, ignore this response\n", fd);
        return RET_FAILED;
    }

    connection->is_validated = true;
    GLOG_INFO("validation status set to %d\n", connection->is_validated);

    return RET_OK;
}

int packet_processor::diagnose_connection(const struct cal::net_connection *input_conn,
    const int input_len,
    struct cal::net_connection **mutable_output_conn,
    int &output_len)
{
    void *in_data_ptr = input_conn->recv_buf->get_read_pointer();
    int in_len = input_len;
    int body_len = CALC_BODY_LEN(in_len);
    int32_t command = get_proto_command(in_data_ptr);
    int32_t out_cmd = 0;
    void *out_data_ptr = (*mutable_output_conn)->send_buf->get_write_pointer();
#ifndef USE_JSON_MSG
    static IdentityReportReq s_id_report_req;
    static IdentityReportResp s_id_report_resp;
#else
    static Json::Value s_id_report_req;
    static Json::Value s_id_report_resp;
#endif
    msg_base *in_body = NULL;
    msg_base *out_body = NULL;
    int retcode = PROTO_RET_SUCCESS;
    int business_ret = -1;
    business_func business_op = NULL;

    output_len = 0;

    switch (command)
    {
    case CMD_HEARTBEAT_REQ:
        business_op = BUSINESS_FUNC(heartbeat_request_handling);
        out_cmd = CMD_HEARTBEAT_RESP;
        break;

    case CMD_HEARTBEAT_RESP:
        business_op = BUSINESS_FUNC(heartbeat_response_handling);
        break;

    case CMD_IDENTITY_REPORT_REQ:
        in_body = &s_id_report_req;
        out_body = &s_id_report_resp;
        business_op = BUSINESS_FUNC(identity_report_request_handling);
        out_cmd = CMD_IDENTITY_REPORT_RESP;
        break;

    case CMD_IDENTITY_REPORT_RESP:
        in_body = &s_id_report_resp;
        business_op = BUSINESS_FUNC(identity_report_response_handling);
        break;

    default:
        GLOG_WARN_C("command code 0x%X is not connection relative\n", command);
        return RET_FAILED;
    }

    if (NULL != in_body && parse_message(GET_BODY_ADDR(in_data_ptr), body_len, *in_body) < 0)
    {
        GLOG_ERROR_C("protocol parse function failed\n");
        return RET_FAILED;
    }

    if (NULL != out_body)
        clear_message_holder(*out_body);

    business_ret = business_op(input_conn, in_body, mutable_output_conn, out_body, retcode);
    if (!proto_is_heartbeat(command))
        GLOG_INFO_C("finished diagnosing the current connection\n");

    if (0 == out_cmd)
        return business_ret;

    return serialize_to_buffer(out_cmd, out_body, in_data_ptr, out_data_ptr, output_len, retcode);
    // for testing only
    //return serialize_to_buffer((CMD_IDENTITY_REPORT_REQ == command) ? CMD_IDENTITY_REPORT_REQ : out_cmd, out_body, in_data_ptr, out_data_ptr, output_len, retcode);
}

void packet_processor::print_supported_commands(void)
{
#ifdef IS_DISPATCHER
    printf("Not supported! Run it's relative operator with this option to view the supported functionalities.\n");
#else
    if (CMD_UNUSED != g_packet_handler_components[0].in_cmd)
        printf("[命令码对应功能列表]\n");
    for (int i = 0; CMD_UNUSED != g_packet_handler_components[i].in_cmd; ++i)
    {
        handler_component &component = g_packet_handler_components[i];

#ifndef USE_JSON_MSG
        printf("0x%08X\t%s(%s)\n", component.in_cmd, component.description,
            (NULL != component.whole_in_body) ? typeid(*(component.whole_in_body)).name()
                : typeid(*(component.partial_in_body)).name());
#else
        printf("0x%08X\t%s\n", component.in_cmd, component.description);
#endif
    }
#endif
}

int packet_processor::get_current_max_packet_length(void)
{
    return m_max_packet_length;
}

void packet_processor::update_max_packet_length(int len)
{
    if (len > m_max_packet_length)
        m_max_packet_length = len;
}

bool packet_processor::is_available(void)
{
    return (NULL != m_message_cache &&
        NULL != m_component_map &&
        NULL != m_timestamps_when_pkts_incomplete);
}

bool packet_processor::is_ready(void)
{
    return (is_available() &&
        !(m_component_map->empty()));
}

int packet_processor::__inner_init(void)
{
    if ((NULL == m_message_cache) &&
        (NULL == (m_message_cache = new message_cache(message_cache::DEFAULT_DICT_SIZE))))
    {
        GLOG_ERROR_C("MessageCache structure initialization failed\n");
        return RET_FAILED;
    }

    if ((NULL == m_component_map) &&
        (NULL == (m_component_map = new component_map)))
    {
        GLOG_ERROR_C("ComponentMap structure initialization failed\n");
        return RET_FAILED;
    }

    if ((NULL == m_timestamps_when_pkts_incomplete) &&
        (NULL == (m_timestamps_when_pkts_incomplete = new std::map<std::string, int64_t>)))
    {
        GLOG_ERROR_C("m_error_timestamps structure initialization failed\n");
        return RET_FAILED;
    }

    /*if ((NULL == m_dispatcher_by_loginid) &&
        (NULL == (m_dispatcher_by_loginid = new LoginDispatcherMap)))
    {
        LOG_ERROR_CV("m_dispatcher_by_loginid structure initialization failed\n");
        return RET_FAILED;
    }*/

    return RET_OK;
}

void packet_processor::__clear(void)
{
    clear_components();

    if (NULL != m_message_cache)
    {
        delete m_message_cache;
        m_message_cache = NULL;
    }

    if (NULL != m_component_map)
    {
        delete m_component_map;
        m_component_map = NULL;
    }

    if (NULL != m_timestamps_when_pkts_incomplete)
    {
        delete m_timestamps_when_pkts_incomplete;
        m_timestamps_when_pkts_incomplete = NULL;
    }

    /*if (NULL != m_dispatcher_by_loginid)
    {
        delete m_dispatcher_by_loginid;
        m_dispatcher_by_loginid = NULL;
    }*/
}

} // namespace cafw

DECLARE_BUSINESS_FUNC(unused_command)
{
    return RET_OK;
}
