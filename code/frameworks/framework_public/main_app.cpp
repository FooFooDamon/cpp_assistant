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

#include "main_app.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <sstream>

#include <google/protobuf/stubs/common.h>
#include <otlv4.h>

#include "customization.h"

#include "base/all.h"
#include "signal_registration.h"
#include "config_manager.h"
#include "resource_manager.h"
#include "timed_task_scheduler.h"
#include "connection_cache.h"
#if defined(HAS_TCP)
#include "message_cache.h"
#include "packet_processor.h"
#endif

#if defined(COMPLEX_APP) && !defined(HAS_CONFIG_FILES)
#error "A complex application requires the HAS_CONFIG_FILES macro and operations for configuration file(s)!"
#endif

extern const calns::command_line::user_option *g_kPrivateOptions;

EXTERN_DECLARE_ALL_CUSTOMIZED_SIG_HANDLER_VARS();

namespace cafw
{

main_app::main_app()
    : m_cmdline(calns::singleton<calns::command_line>::get_instance())
#if defined(HAS_CONFIG_FILES)
    , m_config_manager(calns::singleton<config_manager>::get_instance())
#else
    , m_config_manager(NULL)
#endif
    , m_resource_manager(calns::singleton<resource_manager>::get_instance())
    , m_timed_task_scheduler(calns::singleton<timed_task_scheduler>::get_instance())
#if defined(HAS_TCP)
    , m_packet_processor(calns::singleton<packet_processor>::get_instance())
#else
    , m_packet_processor(NULL)
#endif
{
    ;
}

main_app::~main_app()
{
    release_resources();
}

int main_app::prepare_prerequisites(void)
{
    if (NULL != g_screen_logger)
        return RET_OK;

    try
    {
        g_screen_logger = new calns::screen_logger;
    }
    catch(std::bad_alloc &ex)
    {
        LOGF_C(E, "Failed to create g_screen_logger instance: %s\n", ex.what());
        return RET_FAILED;
    }

    return RET_OK;
}

int main_app::parse_command_line(int argc, char **argv)
{
    calns::command_line *cmdline = m_cmdline;
    calns::command_line::user_option builtin_options[] = {
        {
            /* .full_name = */"h,help",
            /* .description = */HELP_DESC,
            /* .least_value_count = */0,
            /* .value_count_is_fixed = */true,
            /* .assign_expression = */"",
            /* .default_values = */NULL
        },
        {
            /* .full_name = */"v,version",
            /* .description = */VERSION_DESC,
            /* .least_value_count = */0,
            /* .value_count_is_fixed = */true,
            /* .assign_expression = */"",
            /* .default_values = */NULL
        },
#ifdef HAS_CONFIG_FILES
        {
            /* .full_name = */"c,config-file",
            /* .description = */CONFIG_LOADING_DESC,
            /* .least_value_count = */CONFIG_FILE_COUNT,
#ifdef CONFIG_FILE_COUNT_NOT_FIXED
            /* .value_count_is_fixed = */false,
#else
            /* .value_count_is_fixed = */true,
#endif
            /* .assign_expression = */CONFIG_ASSIGN_EXPRESSION,
            /* .default_values = */DEFAULT_CONFIG_FILES
        },
#endif
        {
            /* .full_name = */"d,daemon",
            /* .description = */DAEMON_DESC,
            /* .least_value_count = */0,
            /* .value_count_is_fixed = */true,
            /* .assign_expression = */"",
            /* .default_values = */NULL
        },
        {
            /* .full_name = */"q,quiet-mode",
            /* .description = */QUIET_MODE_DESC,
            /* .least_value_count = */0,
            /* .value_count_is_fixed = */true,
            /* .assign_expression = */"",
            /* .default_values = */NULL
        }
    };
    int builtin_option_count = sizeof(builtin_options) / sizeof(calns::command_line::user_option);
    int learn_ret = cmdline->learn_options(builtin_options, builtin_option_count);

    if (learn_ret < 0)
    {
        LOGF_C(E, "Failed to learn built-in command line options, ret = %d\n", learn_ret);
        return RET_FAILED;
    }

    if (NULL != g_kPrivateOptions)
    {
        int private_option_count = -1;

        while (g_kPrivateOptions[++private_option_count].full_name);

        if ((learn_ret = cmdline->learn_options(g_kPrivateOptions, private_option_count)) < 0)
        {
            LOGF_C(E, "Failed to learn private command line options, ret = %d\n", learn_ret);
            return RET_FAILED;
        }
    }

    cmdline->set_usage_header(USAGE_TITLE);
    cmdline->set_usage_format(USAGE_FORMAT);

    int parse_ret = cmdline->parse(argc, (const char **)argv);
    std::string usage_str;

    if (parse_ret < 0)
    {
        LOGF_C(E, "Failed to parse command line, ret = %d\n", parse_ret);

        std::string parsing_result;

        cmdline->get_parsing_result(&parsing_result);
        fprintf(stderr, "%s\n", parsing_result.c_str());

        RLOGF(W, "See the usage below for reference:\n");
        cmdline->usage(&usage_str);
        fprintf(stderr, "%s\n", usage_str.c_str());

        return RET_FAILED;
    }

    const calns::command_line::option_value_t *opt_entry = NULL;

    opt_entry = cmdline->get_option_value("help");
    if (opt_entry->is_specified)
    {
        cmdline->usage(&usage_str);
        printf("%s\n", usage_str.c_str());
        exit(EXIT_SUCCESS);
    }

    opt_entry = cmdline->get_option_value("version");
    if (opt_entry->is_specified)
    {
        printf("%s version: %s\n", cmdline->program_name(), MODULE_VERSION);
        printf("%s version: %s\n", CASDK_FRAMEWORK_NAME, CASDK_FRAMEWORK_VERSION);
        printf(CPP_ASSISTANT_NAME" library version: %s\n", calns::get_library_version());
#ifdef HAS_PROTOBUF
        printf("Protocol buffer version: v%d.%d.%d\n", GOOGLE_PROTOBUF_VERSION / 1000000,
            (GOOGLE_PROTOBUF_VERSION % 1000000) / 1000, GOOGLE_PROTOBUF_VERSION % 1000);
#endif
#ifdef HAS_DATABASE
        printf("OTL version: v%ld.%ld.%ld\n", (OTL_VERSION_NUMBER & (0xff << 16)) >> 16,
            (OTL_VERSION_NUMBER & (0xf << 12)) >> 12, OTL_VERSION_NUMBER & 0xfff);
#endif
        exit(EXIT_SUCCESS);
    }

    opt_entry = cmdline->get_option_value("quiet-mode");
    if (opt_entry->is_specified)
        enable_quiet_mode();

#ifdef HAS_CONFIG_FILES
    opt_entry = cmdline->get_option_value("config-file");
    if (opt_entry->is_specified && !is_quiet_mode())
    {
        printf("Configuration file(s):");
        for (size_t i = 0; i < opt_entry->values.size(); ++i)
        {
            printf(" [%s]", opt_entry->values[i].c_str());
        }
        printf("\n");
    }
#endif

    bool should_exit = false;

    if (check_private_commandline_options(*cmdline, should_exit) < 0)
    {
        LOGF_C(E, "Failed to check private options\n");
        exit(EXIT_FAILURE);
    }

    if (should_exit)
        exit(EXIT_SUCCESS);

    // This option should be the last one to handle.
    opt_entry = cmdline->get_option_value("daemon");
    if (opt_entry->is_specified)
    {
        printf("Program is about to run as a daemon ...\n");

        // Eliminates memory leaks info caused by exceptional abort.
        // It's safe to be called multiple times
        //
        // wxc, 20160625: DO NOT do this until a process really exits,
        // otherwise, some weird problems may occur, for example:
        // crash due to a parsing by an unmatched protocol object.
        // Memory leak checking programs, e.g. valgrind, may report
        // much annoying info. Just ignore it, since those memory leaks
        // are "possible lost" and "still reachable", which mean that they
        // are not real leaks.
        //
        //google::protobuf::ShutdownProtobufLibrary();

        calns::daemon::daemonize();
    }

    return RET_OK;
}

#if defined(HAS_CONFIG_FILES)
int main_app::load_configurations(void)
{
    const char *config_file = m_cmdline->get_option_value("config-file")->values[0].c_str();

    if (m_config_manager->open_file(config_file) < 0)
    {
        LOGF_C(E, "failed to open %s\n", config_file);
        return RET_FAILED;
    }

    if (m_config_manager->load() < 0)
    {
        LOGF_C(E, "failed to load contents from %s\n",
            m_config_manager->config_content()->config_file_path.c_str());
        return RET_FAILED;
    }

    QLOGF_C(I, "configuration file [%s] was loaded successfully\n", config_file);

    m_config_manager->close_file();

    return RET_OK;
}
#endif

int main_app::prepare_resources(void)
{
    const config_content_t *config = NULL;

    if (NULL != m_config_manager)
        config = m_config_manager->config_content();

    if (m_resource_manager->prepare((config_content_t *)config) < 0)
    {
        LOGF_C(E, "ResourceManager::Prepare() failed\n");
        return RET_FAILED;
    }

#ifdef HAS_TCP
    if (m_packet_processor->build_component_map())
    {
        LOGF_C(E, "PacketProcessor::BuildComponentMap() failed\n");
        return RET_FAILED;
    }
#endif

    return RET_OK;
}

const signal_registration_info g_sig_configs[] = {
    { SIGTERM, "SIGTERM", CUSTOMIZED_SIG_HANDLER(sigterm), true, false },
    { SIGINT, "SIGINT", CUSTOMIZED_SIG_HANDLER(sigint), true, false },
    { SIGPIPE, "SIGPIPE", CUSTOMIZED_SIG_HANDLER(sigpipe), false, false },
    { SIGUSR2, "SIGUSR2", CUSTOMIZED_SIG_HANDLER(sigusr2), false, false },
    { SIGUSR1, "SIGUSR1", CUSTOMIZED_SIG_HANDLER(sigusr1), false, false },
    { SIGCHLD, "SIGCHLD", CUSTOMIZED_SIG_HANDLER(sigchild), false, false },
    { SIGHUP, "SIGHUP", CUSTOMIZED_SIG_HANDLER(sighup), false, false },
    { SIGALRM, "SIGALRM", CUSTOMIZED_SIG_HANDLER(sigalarm), false, false },
    { SIGTRAP, "SIGTRAP", CUSTOMIZED_SIG_HANDLER(sigtrap), false, false },
    { SIGSEGV, "SIGSEGV", CUSTOMIZED_SIG_HANDLER(sigsegv), true, true },
    { calns::INVALID_SIGNAL_NUM, /* The leftover members are not cared."INVALID_SIGNUM", NULL, false, false*/ }
};

int main_app::register_signals(void)
{
    int ret = CA_RET_GENERAL_FAILURE;

    for (int i = 0; calns::INVALID_SIGNAL_NUM != g_sig_configs[i].sig_num; ++i)
    {
        const signal_registration_info &sig_config = g_sig_configs[i];

        ret = calns::sigcap::register_one(sig_config.sig_num,
            sig_config.sig_func, sig_config.exit_after_handling,
            sig_config.handles_now);

        if (CA_RET_OK != ret)
        {
            LOGF_C(E, "failed to register %s, ret = %d\n", sig_config.sig_name, ret);
            return RET_FAILED;
        }

        QLOGF_C(I, "signal{ name[%s] | num[%d] | exit_after_handling[%d] | handles_now[%d] } registered\n",
            sig_config.sig_name, sig_config.sig_num, sig_config.exit_after_handling, sig_config.handles_now);
    }

    return RET_OK;
}

timed_task_info_t g_built_in_timed_tasks[] = {
    /*
     * {
     *      task name,
     *      { trigger_type, has_triggered, event_time/last_op_time[us], time_offset/time_interval[ms], operation }
     * }
     */

    {
        XNODE_MSG_CLEAN,
        { timed_task_config::TRIGGERED_PERIODICALLY,   true,   {0},    {1000},    default_message_clean_timed_task }
    },
    {
        XNODE_SESSION_CLEAN,
        { timed_task_config::TRIGGERED_PERIODICALLY,   true,   {0},    {1000},    default_session_clean_timed_task }
    },
    {
        XNODE_HEARTBEAT,
        { timed_task_config::TRIGGERED_PERIODICALLY,   true,   {0},    {1000},    default_heartbeat_timed_task }
    },
    {
        XNODE_LOG_FLUSHING,
        { timed_task_config::TRIGGERED_PERIODICALLY,   true,   {0},    {1000},    default_log_flushing_timed_task }
    },
    {
        NULL,
        {}
    }
};

int main_app::register_timed_tasks(void)
{
    timed_task_info_t *task_items[] = {
        g_built_in_timed_tasks,
        g_customized_timed_tasks
    };

    for (unsigned int i = 0; i < 2; ++i)
    {
        for (int j = 0; NULL != task_items[i][j].name; ++j)
        {
            timed_task_config &task_config = task_items[i][j].config;
            const char *TASK_NAME = task_items[i][j].name;

            if (timed_task_config::TRIGGERED_PERIODICALLY == task_config.trigger_type)
            {
#ifdef HAS_CONFIG_FILES
                int64_t time_interval = m_config_manager->get_time_interval_in_microseconds(TASK_NAME);

                if (RET_FAILED == time_interval)
                {
                    LOGF_C(W, "can not find interval value for %s\n", TASK_NAME);
                    if (g_built_in_timed_tasks == task_items[i])
                        return RET_FAILED;

                    LOGF_C(I, "use the value in the code\n");
                }
                else
                    task_config.time_interval = time_interval / 1000; // switched to millisecond
#endif
            }
            else
            {
                task_config.event_time = 0; // will be refreshed in init_business() or in a specific function
                task_config.time_offset = 0; // TODO: gets value from config or within init_business()
            }

            if (m_timed_task_scheduler->register_one(TASK_NAME, task_config) < 0)
                return RET_FAILED;

            QLOGF_C(I, "timed task{ name[%s] | trigger_type[%s] | event_time/last_op_time[%ld us]"
                " | time_offset/time_interval[%ld ms] } registered successfully\n",
                TASK_NAME, timed_task_scheduler::get_trigger_type_description(task_config.trigger_type),
                task_config.event_time, task_config.time_offset);
        }
    }

    return RET_OK;
}

int main_app::initialize_business(int argc, char **argv)
{
    return init_business(argc, argv);
}

void main_app::finalize_business(void)
{
    ::finalize_business();
}

int main_app::run_business(void)
{
    int ret = RET_FAILED;
#if defined(HAS_TCP)

#if defined(HAS_UPSTREAM_SERVERS)
    calns::tcp_client *client_requester = m_resource_manager->resource()->client_requester;
#endif

#if defined(ACCEPTS_CLIENTS)
    calns::tcp_server *server_listener = m_resource_manager->resource()->server_listener;
#endif // defined(ACCEPTS_CLIENTS)

#endif // defined(HAS_TCP)

    fprintf(stdout, "%s started successfully, version: %s, pid: %d\n",
        m_cmdline->program_name(), MODULE_VERSION, getpid());

    while (true)
    {
        if (expires())
        {
            LOGF_C(E, "program has expired, exits now\n");
            break;
        }

        ret = RET_OK;

        bool should_exit = false;

        calns::sigcap::handle_all(should_exit);
        if (should_exit)
        {
            LOGF_C(W, "critical signals captured, process about to exit !!!\n");
            break;
        }

#if defined(HAS_TCP)
#if defined(ACCEPTS_CLIENTS)
        poll_and_process(server_listener, should_exit);
        if (should_exit) break;
#endif
#if defined(HAS_UPSTREAM_SERVERS)
        poll_and_process(client_requester, should_exit);
        if (should_exit) break;
#endif
#endif // defined(HAS_TCP)

        m_timed_task_scheduler->check_and_execute();

        ret = run_private_business(should_exit);
        if (should_exit) break;
    }

    return ret;
}

void main_app::release_resources(void)
{
    if (NULL != m_resource_manager)
        m_resource_manager->clean();

    /*if (NULL != g_screen_logger)
    {
        delete g_screen_logger;
        g_screen_logger = NULL;
    }*/
#ifdef HAS_PROTOBUF
    google::protobuf::ShutdownProtobufLibrary();
#endif
}

bool main_app::expires(void)
{
#ifdef CHECKS_EXPIRATION
    int64_t cur_time = calns::TimeHelper::GetUtcMicroseconds();

    return (cur_time >= m_config_manager->config_entries()->private_configs.expiration);
#else
    return false;
#endif
}

#if defined(HAS_TCP)

void main_app::poll_and_process(calns::tcp_base *tcp_manager, bool &should_exit)
{
    int active_peer_count = tcp_manager->poll();
    calns::tcp_base::conn_info_array *active_peer_array = &(tcp_manager->get_active_peers());
    const int kMaxHandleCountPerCycle = CFG_GET_COUNTER(XNODE_MSG_PROCESS_COUNT_PER_ROUND);
    const int kSendBufSize = CFG_GET_BUF_SIZE(XNODE_TCP_SEND_BUF);
    const int kRecvBufSize = CFG_GET_BUF_SIZE(XNODE_TCP_RECV_BUF);

    for (int i = 0; i < active_peer_count; ++i)
    {
        int in_fd = ((calns::net_connection*)(active_peer_array->elements[i].data.ptr))->fd;

        if (tcp_manager->listening_fd() == in_fd)
        {
            calns::tcp_server *tcp_server = dynamic_cast<calns::tcp_server *>(tcp_manager);

            if (NULL == tcp_server)
            {
                LOGF_C(W, "this connection node is not a server,"
                    " can not accept a connection, fd = %d\n", in_fd);
                continue;
            }

            accept_new_connection(tcp_server, kSendBufSize, kRecvBufSize);
        }
        else
        {
            calns::net_connection *conn = (calns::net_connection*)(active_peer_array->elements[i].data.ptr);
            int recv_ret = tcp_manager->recv_to_connection(conn);

            if (recv_ret < 0)
            {
                LOGF_C(E, "failed to received packets and put them into connection[%d],"
                    " ret = %d, err = %s\n", conn->fd, recv_ret, calns::what(recv_ret).c_str());

                if (CA_RET(CONNECTION_BROKEN) == recv_ret)
                {
                    shut_bad_connection(tcp_manager, conn);
                    continue;
                }
            }
            else
            {
                if (recv_ret > 0)
                    RLOGF(D, "%d bytes received\n", recv_ret);
            }

            if (conn->recv_buf->empty())
                continue;

            handle_received_packets(conn, kMaxHandleCountPerCycle);
        }
    }

    send_result_packets(tcp_manager);
}

int main_app::accept_new_connection(calns::tcp_server *tcp_server, int send_buf_size, int recv_buf_size)
{
    int accfd = tcp_server->accept_new_connection(send_buf_size, recv_buf_size);

    if (accfd < 0)
    {
        LOGF_C(E, "failed to accept new connection, ret = %d, err = %s\n", accfd, calns::what(accfd).c_str());

        return RET_FAILED;
    }

    calns::tcp_base::conn_map *all_peers = tcp_server->peers();
    calns::net_connection *new_conn = (*all_peers)[accfd];

    snprintf(new_conn->self_name, sizeof(new_conn->self_name), "%s", tcp_server->self_name());

    RLOGF(I, "new connection[%d] arrived: fd[%d] | self_ip[%s] | self_port[%u] | self_name[%s]"
        " | peer_ip[%s] | peer_port[%u] | peer_name[%s] | status[0x%08X]"
        " | is_blocking[%d] | is_validated[%d] | send_buffer[%d bytes] | recv_buffer[%d bytes]\n",
        accfd, new_conn->fd, new_conn->self_ip, new_conn->self_port, new_conn->self_name,
        new_conn->peer_ip, new_conn->peer_port, new_conn->peer_name, new_conn->conn_status,
        new_conn->is_blocking, new_conn->is_validated, new_conn->send_buf->total_size(), new_conn->recv_buf->total_size());

    return RET_OK;
}

void main_app::shut_bad_connection(calns::tcp_base *tcp_manager, calns::net_connection *bad_conn)
{
    LOGF_C(W, "peer shut down, fd = %d, name = %s, ip = %s, port = %u\n",
        bad_conn->fd, bad_conn->peer_name, bad_conn->peer_ip, bad_conn->peer_port);

    dict_entry_ptr owner = (dict_entry_ptr)(bad_conn->owner);
    net_conn_index* conn_index = NULL;

    if (NULL != owner
        && NULL != (conn_index = (net_conn_index*)GET_CHAR_DICT_VALUE(owner)))
    {
        RLOGF(I, "found connection cache info of this node, name is [%s],"
            " cleaned up cache info\n", GET_CHAR_DICT_KEY(owner));
        conn_index->fd = calns::INVALID_SOCK_FD;
        conn_index->conn_detail = NULL;
    }

    tcp_manager->shutdown_connection(bad_conn);
    RLOGF(I, "finished releasing connection resource at local end\n");
}

void main_app::handle_received_packets(calns::net_connection *input_conn, int max_packet_count)
{
    calns::buffer *in_buf = input_conn->recv_buf;
    int handle_count = 0;

    while (!(in_buf->empty()) && handle_count < max_packet_count)
    {
        RLOGF(D, "handling packets from input connection{ fd[%d] | name[%s] }.recv_buf: %d bytes pending,"
            " current read position = %d, write position = %d\n",
            input_conn->fd, input_conn->peer_name, in_buf->data_size(),
            in_buf->read_position(), in_buf->write_position());

        int bytes_handled = 0;
        int bytes_output = 0;
        calns::net_connection **mutable_output_conn = &(input_conn);
        int ret = m_packet_processor->process(input_conn, bytes_handled, mutable_output_conn, bytes_output);

        if (CA_RET(RESOURCE_NOT_AVAILABLE) == ret
            || CA_RET(OPERATION_TIMED_OUT) == ret
            || CA_RET(LENGTH_TOO_BIG) == ret)
        {
            LOGF_C(W, "packet processing aborted due to abnormal data\n");
            break;
        }

        ++handle_count;

        if (bytes_handled > 0)
        {
            in_buf->move_read_pointer(bytes_handled);
            input_conn->last_op_time = calns::time_util::get_utc_microseconds();
        }

        RLOGF(D, "%d bytes input handled, %d bytes output generated\n", bytes_handled, bytes_output);

        if (bytes_output <= 0)
            continue;

        calns::net_connection *actual_output_conn = (*mutable_output_conn);
        calns::buffer *out_buf = actual_output_conn->send_buf;

        out_buf->move_write_pointer(bytes_output);
        actual_output_conn->last_op_time = calns::time_util::get_utc_microseconds();

        RLOGF(D, "loading packets into output connection{ fd[%d] | name[%s] }.send_buf: %d bytes free,"
            " current read position = %d, write position = %d\n",
            actual_output_conn->fd, actual_output_conn->peer_name, out_buf->free_size(),
            out_buf->read_position(), out_buf->write_position());
    }

    if (handle_count > 0)
        RLOGF(D, "%d messages handled during this round\n", handle_count);
}

void main_app::send_result_packets(calns::tcp_base *tcp_manager)
{
    calns::tcp_base::conn_map *all_peers = tcp_manager->peers();

    for (calns::tcp_base::conn_map::iterator iter = all_peers->begin(); iter != all_peers->end(); ++iter)
    {
        calns::net_connection *conn = iter->second;

        if (NULL == conn)
            continue;

        if (conn->send_buf->empty())
            continue;

        int ret = 0;

        if ((ret = tcp_manager->send_from_connection(conn)) < 0)
        {
            LOGF_C(E, "failed to send contents of connection[%d], ret = %d, err = %s\n",
                iter->first, ret, calns::what(ret).c_str());
        }
        else
            RLOGF(D, "%d bytes sent\n", ret);
    }
}

#endif // defined(HAS_TCP)

}
