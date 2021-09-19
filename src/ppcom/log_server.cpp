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


void*
log_serv_cmd_handler(message_param *param)
{
	message_param *msg_param = param;

	if (param == NULL) {
		loge("message param is NULL");
		return NULL;
	}
	// if (msg_param->sub_param == NULL)
	//     loge("custon param is NULL");

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

	logd("[%s:%d] send message: %s", __func__, __LINE__, msg_param->cmd);
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
			logd("async sent msg %d, len:[%d] : %s", i0, strlen(buffer), buffer);
			write(msockfd, buffer, strlen(buffer) + 1);
		}

		i0++;
	}
	close(msockfd);
	return 0;
}

void start_log_server(void* param)
{
	process_manager procmgr;
	message_param *msg_param;


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


	msg_param = serv_mgr.get_message_manager_class()->get_msg_serv_param();
	serv_mgr.register_serv_msg_callback(log_serv_cmd_handler, msg_param);

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

