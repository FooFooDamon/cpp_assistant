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

#include "signal_registration.h"

#include <sstream>

#include "base/all.h"

#include "config_manager.h"
#include "connection_cache.h"
#include "resource_manager.h"
#include "timed_task_scheduler.h"

namespace cafw
{

DECLARE_DEFAULT_SIG_HANDLER(sigusr2)
{
    GLOG_INFO("sigusr2 received, flushing log ...\n");
    default_log_flushing_timed_task();
    return RET_OK;
}

DECLARE_DEFAULT_SIG_HANDLER(sighup)
{
#ifdef HAS_CONFIG_FILES
    GLOG_INFO("sighup received, reloading partial configurations ...\n");
    return cal::singleton<config_manager>::get_instance()->reload_partial();
#else
    GLOG_INFO("sighup received, do nothing\n");
    return RET_OK;
#endif
}

DECLARE_DEFAULT_SIG_HANDLER(sigusr1)
{
    GLOG_INFO("sigusr1 received, outputing some debug info ...\n");
    GLOG_INFO("debug info begins ==========================================>\n");

#ifdef HAS_TCP
    const resource_t *resource = cal::singleton<resource_manager>::get_instance()->resource();

    struct conn_cache_info
    {
        connection_cache *ptr;
        const char *name;
    } conn_cache_items[] = {
        { resource->master_connection_cache, "master connection cache"},
        { resource->slave_connection_cache, "slave connection cache"}
    };

    /*
     * info of connection cache
     */
    for (size_t i = 0; i < sizeof(conn_cache_items) / sizeof(struct conn_cache_info); ++i)
    {
        GLOG_INFO("---- %s:\n", conn_cache_items[i].name);
        conn_cache_items[i].ptr->profile();
    }

    /*
     * info of tcp managers
     */

    std::string conn_info;
    std::istringstream info_stream;
    std::string line;
    struct mgrinfo
    {
        cal::tcp_base *tcp_manager;
        const char *manager_name;
    } manager_info[] = {
        { resource->server_listener, "server listener" },
        { resource->client_requester, "client requester" }
    };

    for (size_t i = 0; i < sizeof(manager_info) / sizeof(struct mgrinfo); ++i)
    {
        manager_info[i].tcp_manager->has_what(&conn_info);
        info_stream.str(conn_info);
        GLOG_INFO("---- details of %s:\n", manager_info[i].manager_name);
        while (std::getline(info_stream, line))
        {
            GLOG_INFO("%s\n", line.c_str());
        }
        info_stream.clear(); // TODO: clears and refreshes some inner flags relative to conn_info ??
    }
#endif

    GLOG_INFO("<========================================== debug info ends\n");

    /*
     * flushes info above immediately
     */
    GLOG_FLUSH();

    return RET_OK;
}

DECLARE_DEFAULT_SIG_HANDLER(sigsegv)
{
    GLOG_WARN("sigsegv received, flushing log ...\n");
    GLOG_WARN("Program will exit abnormally and generate a core file containing crash info!!!\n");
    GLOG_FLUSH();
    // TODO: notify worker threads
    cal::sigcap::unregister(SIGSEGV);
    //signal(SIGSEGV, SIG_DFL); // the simpler but unsafe method of canceling registration
    raise(SIGSEGV);
    return RET_OK;
}

DEFINE_DEFAULT_SIG_HANDLER(sigterm);
DEFINE_DEFAULT_SIG_HANDLER(sigint);
DEFINE_DEFAULT_SIG_HANDLER(sigpipe);
//DEFINE_DEFAULT_SIG_HANDLER(sigusr1);
DEFINE_DEFAULT_SIG_HANDLER(sigchild);
DEFINE_DEFAULT_SIG_HANDLER(sigalarm);
DEFINE_DEFAULT_SIG_HANDLER(sigtrap);

}
