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

#include <basic.h>

#include <log_server.h>
#if 1 //def CONFIG_LIB_SRVMGR

// new thread conten
service_manager mserv;

extern socket_server_init msocket_server_init;

void*
log_serv_cmd_handler(void *param)
{
	message_param *msg_param = (message_param*)param;

	if (param == NULL) {
		ploge("message param is NULL");
		return NULL;
	}

	if (strcmp(msg_param->cmd, "ack") == 0) {
		sprintf(msg_param->cmd, "%s %s", msg_param->cmd, "ok");
	} else if (strcmp(msg_param->cmd, "quit") == 0) {
		sprintf(msg_param->cmd, "%s %s", msg_param->cmd, "QUIT");
	} else {
		sprintf(msg_param->cmd, "%s %s", msg_param->cmd, "NG");
		return NULL;
	}

	return NULL;
}

int
send_request_cmd(message_param *param)
{
	int msockfd = -1;
	int i0;
	int rlen;
	int wlen;
	char buffer[READ_BUFFER_LEN];
	int ret;
	int fd_max;
	fd_set rd_set;
	message_param *msg_param = param;

	msockfd = msg_param->sockfd;
	// fd_max = MAX_VAL(msockfd, STDIN_FILENO)  + 1;
	fd_max = STDIN_FILENO + 1;

	i0 = 0;

	plogd("[%s:%d] send message: %s", __func__, __LINE__, msg_param->cmd);
	wlen = write(msockfd, msg_param->cmd, strlen(msg_param->cmd)+1);
	while(true) {
		FD_ZERO(&rd_set);

		// FD_SET(msockfd      , &rd_set);
		FD_SET(STDIN_FILENO , &rd_set);

		ret = select(fd_max, &rd_set, NULL, NULL , NULL);
		if (ret < 0 )
			return -EINVAL;

		if (FD_ISSET(STDIN_FILENO, &rd_set))
		{
			memset(buffer,0x0,sizeof(buffer));
			scanf("%s", buffer);
			plogd("async sent msg %d, len:[%d] : %s", i0, strlen(buffer), buffer);
			write(msockfd, buffer, strlen(buffer) + 1);
		}

		i0++;
	}
	close(msockfd);
	return 0;
}

void start_log_server(void* param)
{
	int flag = FLAG_DEFAULT;
	serv_conf mserv_conf;

	msocket_server_init.set_default_socket_server_path("/tmp/ppcom");
#if 1
	// process
	process_manager mserv_proc;
	proc_config pconfig;
	message_param   mmsg;

	//setup serv msg callback
	mserv.set_service_type(MGR_SERVICE_SERV);
	mserv.register_serv_msg_callback(log_serv_cmd_handler, NULL);

	// set process_manager mlock to child thread
	mserv_conf.serv_cls        = (void*)&mserv;

	// *********************************
	pconfig.set_proc_serv(&mserv_conf);
	pconfig.set_proc_mutex(mserv_proc.get_thread_mutex());
	pconfig.set_proc_flag(flag|
		FLAG_WITH_PTHREAD|FLAG_SYNC_MUTEX);
	pconfig.set_proc_param((void*)&mmsg);
	// *********************************

	misc_utils mmisc;
	mmisc.flag_to_string(pconfig.get_proc_flag(), __FUNCTION__);

	mserv_proc.start_routine = mserv.start_service_manager_proc;
	mserv_proc.start_thread_sync_mutex((void*)pconfig.get_proc_conf());

#endif
	// register_serv_handler(void *(*proc_func)(void* param), void* param)
	plogd("============== start_service_manager_proc <done> =================");

}

void stop_log_server(void* param)
{
	plogd("============== stop_service_manager_proc <done> =================");
	return;
}

#else
void start_log_server(void* param)
{
	return;
}
void stop_log_server(void* param)
{
	return;
}
#endif

