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
	memset(mserv_conf.socket_path, 0x0,sizeof(mserv_conf.socket_path));
	while (true)
	{
		ch = getopt(argc, argv, "tip:hs:");
		if (ch < 0)
			break;
		logd("optind: %d, ch: %#x\n", optind, ch);
		switch (ch)
		{
		case 't':
			logd("Start in pthread mode\n");
			flag |= FLAG_WITH_PTHREAD;
			break;
		case 'i':
			logd("Start in ip mode\n");
			flag |= FLAG_WITH_IP;
			break;
		case 'p':
			logd("Start with IP port %s\n", optarg);
			mserv_conf.port = atoi(optarg);
			break;
		case 's':
			logd("Start with socket %s\n", optarg);
			memset(mserv_conf.socket_path, 0x0,sizeof(mserv_conf.socket_path));
			sprintf(mserv_conf.socket_path, "%s", optarg);
			break;
		case 'h':
			// srvmgr_print_usage();
			exit(0);
		}
	}

#if 0
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

#endif
#if 1
	process_manager mcli_proc;
	proc_conf   mcli_proc_param;

	service_manager mserv;
	message_param   mmsg;

	memset(mmsg.cmd, 0x0, sizeof(mmsg.cmd));
	sprintf(mmsg.cmd, "%s", "send_file sample.tar");

	//setup serv msg callback
	mserv.set_service_type(MGR_SERVICE_CLI);
	mserv.register_cli_recv_msg_callback(recv_handler, (void*)"ext byJames");
	mserv.register_cli_send_msg_callback(request_cmd_once, &mmsg);

	// set process_manager mlock to child thread
	mserv_conf.serv_cls    = (void*)&mserv;

	mcli_proc_param.mlock  = mcli_proc.get_thread_mutex();
	mcli_proc_param.flag   = flag;
	mcli_proc_param.flag  |= FLAG_WITH_PTHREAD;
	mcli_proc_param.flag  |= FLAG_BLOCK;
	mcli_proc_param.serv   = &mserv_conf;
	mcli_proc_param.param  = (void*)&mmsg;

	mcli_proc.start_routine = mserv.start_service_manager_proc;
	mcli_proc.start_thread_sync_mutex((void*)&mcli_proc_param);

#endif
	// while (true) {
	//     sleep(120);
	// }
	return 0;

}


