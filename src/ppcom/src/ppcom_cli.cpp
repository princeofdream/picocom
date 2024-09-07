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


process_manager   mcli_proc;
proc_conf         mcli_proc_conf;
serv_conf         mserv_conf;
service_manager   mserv_mgr;
message_param     mmsg;

void srvmgr_print_usage(const char *pname)
{
    plogw("%s -s /tmp/proc-x.sock\n", pname);
}

int main(int argc, char *argv[])
{
	int flag = FLAG_DEFAULT;
	int ch;

	mserv_conf.port = 0;
	memset(mserv_conf.socket_path, 0x0,sizeof(mserv_conf.socket_path));
	while (true)
	{
		ch = getopt(argc, argv, "tip:hs:");
		if (ch < 0)
			break;
		plogd("optind: %d, ch: %#x\n", optind, ch);
		switch (ch)
		{
		case 't':
			plogi("Start in pthread mode\n");
			flag |= FLAG_WITH_PTHREAD;
			break;
		case 'i':
			plogi("Start in ip mode\n");
			flag |= FLAG_WITH_IP;
			break;
		case 'p':
			plogi("Start with IP port %s\n", optarg);
			mserv_conf.port = atoi(optarg);
			break;
		case 's':
			plogi("Start with socket %s\n", optarg);
			memset(mserv_conf.socket_path, 0x0,sizeof(mserv_conf.socket_path));
			sprintf(mserv_conf.socket_path, "%s", optarg);
			break;
		case 'h':
            srvmgr_print_usage(argv[0]);
			exit(0);
		}
	}


	memset(mmsg.cmd, 0x0, sizeof(mmsg.cmd));
	sprintf(mmsg.cmd, "%s", "send_file sample.tar");

	//setup serv msg callback
	mserv_mgr.set_service_type(MGR_SERVICE_CLI);
	mserv_mgr.ml_serv.set_serv_socket_path(mserv_conf.socket_path);
	// mserv_mgr.register_cli_recv_msg_callback(recv_handler, (void*)"ext byJames");
	// mserv_mgr.register_cli_send_msg_callback(request_cmd_once, &mmsg);

	// set process_manager mlock to child thread
	mserv_conf.serv_cls    = (void*)&mserv_mgr;

	mcli_proc_conf.mlock    = mcli_proc.get_thread_mutex();
	mcli_proc_conf.flag     = flag;
	mcli_proc_conf.flag    |= FLAG_WITH_PTHREAD;
	mcli_proc_conf.flag    |= FLAG_BLOCK;
	mcli_proc_conf.servcfg  = &mserv_conf;
	mcli_proc_conf.param    = (void*)&mmsg;

	mcli_proc.start_routine = mserv_mgr.start_service_manager_proc;
	mcli_proc.start_thread_sync_mutex((void*)&mcli_proc_conf);

	// while (true) {
	//     sleep(120);
	// }
	return 0;

}


