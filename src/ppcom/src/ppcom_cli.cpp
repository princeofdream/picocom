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
#include <basic.h>



int main(int argc, char *argv[])
{
	int flag = FLAG_DEFAULT;
	int ch;
	serv_conf mserv_conf;

	mserv_conf.port = 0;
	while (true)
	{
		ch = getopt(argc, argv, "tsp:h");
		if (ch < 0)
			break;
		logd("optind: %d, ch: %#x\n", optind, ch);
		switch (ch)
		{
		case 't':
			logd("Start in pthread mode\n");
			flag |= FLAG_WITH_PTHREAD;
			break;
		case 's':
			logd("Start in ip mode\n");
			flag |= FLAG_WITH_IP;
			break;
		case 'p':
			logd("Start with IP port %s\n", optarg);
			mserv_conf.port = atoi(optarg);
			break;
		case 'h':
			// srvmgr_print_usage();
			exit(0);
		}
	}

	// process
	process_manager mserv_proc;
	proc_config pconfig;
	message_param   mmsg;

	// new thread content
	service_manager mserv;

	//setup serv msg callback
	mserv.set_service_type(MGR_SERVICE_SERV);
	// mserv.register_serv_msg_callback(serv_respond_cmd, NULL);

	// set process_manager mlock to child thread
	mserv_conf.serv_cls        = (void*)&mserv;

	// *********************************
	pconfig.set_proc_serv(&mserv_conf);
	pconfig.set_proc_mutex(mserv_proc.get_thread_mutex());
	pconfig.set_proc_flag(flag|
		FLAG_WITH_PTHREAD|FLAG_BLOCK|FLAG_SYNC_MUTEX);
	pconfig.set_proc_param((void*)&mmsg);
	// *********************************

	misc_utils mmisc;
	mmisc.flag_to_string(pconfig.get_proc_flag(), __FUNCTION__);

	mserv_proc.start_routine = mserv.start_service_manager_proc;
	mserv_proc.start_thread_sync_mutex((void*)pconfig.get_proc_conf());
	// FLAG_BLOCK will block thread unti it quit

	// while (true) {
	//     sleep(120);
	// }
	return 0;

}


