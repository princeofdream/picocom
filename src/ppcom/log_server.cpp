/*
 * =====================================================================================
 *
 *       Filename:  log_server.cpp
 *
 *    Description:  log server
 *
 *        Version:  1.0
 *        Created:  2021年09月18日 13时49分34秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  James Lee (JamesL), princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#include <log_server.h>
#ifdef CONFIG_LIB_SRVMGR

serv_param m_servparam;
process_param m_procparam;

service_manager serv_mgr;

void start_log_server(void* param)
{
	process_manager procmgr;


	// m_servparam.port = 8112;
	memset(m_servparam.socket_path, 0x0, sizeof(m_servparam.socket_path));

	// default is server
	// serv_mgr.set_service_type(MGR_SERVICE_CLI);
	// serv_mgr.set_service_type(MGR_SERVICE_SERV);

	m_servparam.serv_cls = &serv_mgr;
	// m_servparam.serv_cls = NULL;

	m_procparam.flags     = FLAG_WITH_PTHREAD;
	// m_procparam.flags    |= FLAG_WITH_IP;
	// m_procparam.flags    |= FLAG_BLOCK;
	m_procparam.serv      = &m_servparam;
	m_procparam.sub_param = NULL;


	// serv_mgr.register_serv_msg_callback(void *(*proc_func)(void* param), void *param)

	procmgr.start_routine=start_service_manager_proc;
	procmgr.start_thread(&m_procparam);

	// register_serv_handler(void *(*proc_func)(void* param), void* param)
	logd("============== start_service_manager_proc <done> =================");

}

void stop_log_server(void* param)
{
	logd("============== stop_service_manager_proc <done> =================");
	return;
}

#else
void start_log_server(void* param)
{
	// pinfo("=============== no log server build-in ====================\r\n");
	return;
}
void stop_log_server(void* param)
{
	// pinfo("=============== no log server build-in ====================\r\n");
	return;
}
#endif

