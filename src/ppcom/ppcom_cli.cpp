/*
 * =====================================================================================
 *
 *       Filename:  ppcom_cli.cpp
 *
 *    Description: ppcom cli
 *
 *        Version:  1.0
 *        Created:  2021年09月17日 14时31分32秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  James Lee (JamesL), princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#include <process_manager.h>
#include <service_manager.h>



int main()
{
	process_manager procmgr;
	service_manager svcmgr;

	process_param m_procparam;
	serv_param m_servparam;


	svcmgr.set_service_type(MGR_SERVICE_CLI);

	m_servparam.serv_cls = &svcmgr;
	// m_servparam.port = 8112;
	m_servparam.port = 8000;
	memset(m_servparam.socket_path, 0x0, sizeof(m_servparam.socket_path));

	m_procparam.flags = FLAG_WITH_PTHREAD;
	// m_procparam.flags |= FLAG_BLOCK;
	// m_procparam.flags |= FLAG_WITH_IP;
	// m_procparam.flags |= FLAG_SYNC_MUTEX;
	m_procparam.sub_param = NULL;

	m_procparam.mlock = procmgr.get_thread_mutex();
	m_procparam.serv = &m_servparam;

	procmgr.start_routine = start_service_manager_proc;
	procmgr.start_thread(&m_procparam);

	while(1)
	{
		sleep(1);
	}
	return 0;
}


