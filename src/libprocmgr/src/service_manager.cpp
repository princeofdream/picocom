/*
 * =====================================================================================
 *
 *       Filename:  service_manager.cpp
 *
 *    Description:  diagnosis server
 *
 *        Version:  1.0
 *        Created:  03/18/2019 11:12:01 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#include <service_manager.h>
#include <netlink_event.h>


// NOTE
// 1. service_manager setup message callback and send/recv callback
// 2. service_manager setup and socket_server server/client
// 3. local service register process routine
// 4. local service start process by process_manager


service_manager::service_manager()
{
	service_type = MGR_SERVICE_SERV;
	cli_send_msg_callback = NULL;
	pthread_mutex_init(&sync_lock, NULL);
	cli_recv_ready = 0;
}

service_manager::~service_manager()
{
}

void*
service_manager::start_service_manager_proc(void* param)
{
	int mfd;
	static netlink_event mevent;
	service_manager *mserv = NULL;
	proc_conf *mparam = (proc_conf*)param;
	proc_conf mproc_param;
	serv_conf *mserv_conf = NULL;

	if (mparam == NULL) {
		ploge("%s param is null, abort!", __FUNCTION__);
		return (void*)NULL;
	}

	if (mparam->servcfg != NULL) {
		mserv_conf = mparam->servcfg;
		if (mserv_conf->serv_cls != NULL)
			mserv = (service_manager*)mparam->servcfg->serv_cls;
		else
			mserv = new service_manager();
	} else {
		mserv = new service_manager();
	}

	// set process_manager mlock to child thread
	if (mserv_conf != NULL)
		mserv_conf->serv_cls = mserv;

	mproc_param.mlock = mparam->mlock;
	mproc_param.flag  = mparam->flag;
	mproc_param.servcfg  = mserv_conf;
	mproc_param.param = mparam->param;

	// misc_utils mmisc;
	// mmisc.flag_to_string(mproc_param.flag, __func__);

	memset(mproc_param.cmd, 0x0, sizeof(mproc_param.cmd));
	sprintf(mproc_param.cmd, "%s", mparam->cmd);

	if (mserv->service_type == MGR_SERVICE_SERV)
		mserv->start_manager_server(&mproc_param);
	else if (mserv->service_type == MGR_SERVICE_CLI)
		mserv->start_client_manager(&mproc_param);
	else
		plogd("service type incorrect, Do not start manager service.\n");

	plogd("service done, quit...");
	if((mparam->servcfg != NULL) && (mserv_conf->serv_cls == NULL) && (mserv != NULL)) {
		delete(mserv);
		mserv = NULL;
	} else if((mparam->servcfg == NULL) && (mserv != NULL)) {
		delete(mserv);
		mserv = NULL;
	}
	return (void*)NULL;
}

void*
start_client_manager_proc(void* param)
{
	int mfd = -1;
	netlink_event mevent;
	proc_conf *mparam = (proc_conf*)param;
	service_manager *mcli = NULL;
	message_param *req_msg_param;

	if (mparam == NULL) {
		ploge("%s param is null, abort!", __FUNCTION__);
		return (void*)NULL;
	}

	if (mparam->servcfg != NULL) {
		mcli = (service_manager*)mparam->servcfg->serv_cls;
	} else {
		mcli = new service_manager();
	}

	if (mparam->param != NULL)
		req_msg_param = (message_param*)mparam->param;
	else
		req_msg_param = new message_param;

	mfd = dup(mparam->listenfd);

	req_msg_param->sockfd = mfd;
	req_msg_param->param = mcli->req_cli_param;

	if (strlen(req_msg_param->cmd) == 0) {
		memset(req_msg_param->cmd, 0x0, sizeof(req_msg_param->cmd));
		sprintf(req_msg_param->cmd, "%s", "ack");
	}

	if (((mparam->flag & FLAG_SYNC_MUTEX) == FLAG_SYNC_MUTEX) && (mparam->mlock != NULL)) {
		plogd("[mutex] unlock prarent thread <%s:%d>", __FUNCTION__,__LINE__);
		pthread_mutex_unlock(mparam->mlock);
	}

	// waiting recv ready
	while (mcli->cli_recv_ready < 0) {
		usleep(500);
	}
	if (mcli->cli_send_msg_callback != NULL) {
		mcli->cli_send_msg_callback(req_msg_param);
	} else {
		// mcli->send_request_cmd_once(req_msg_param);
		mcli->send_request_cmd(req_msg_param);
	}

	close(mfd);

	if ((mparam->servcfg == NULL) && (mcli != NULL)) {
		delete(mcli);
		mcli = NULL;
	}

	if ((mparam->param == NULL) && (req_msg_param != NULL)) {
		delete req_msg_param;
		req_msg_param = NULL;
	}

	return (void*)NULL;
}

void*
start_client_manager_recv_proc(void* param)
{
	int mfd = -1;
	netlink_event mevent;
	proc_conf *mparam = (proc_conf*)param;
	service_manager *mcli = NULL;
	message_param recv_param;

	if (mparam == NULL) {
		ploge("%s param is null, abort!", __FUNCTION__);
		return (void*)NULL;
	}

	if (mparam->servcfg != NULL) {
		mcli = (service_manager*)mparam->servcfg->serv_cls;
	} else {
		mcli = new service_manager();
	}

	mfd = dup(mparam->listenfd);

	recv_param.sockfd = mfd;
	recv_param.param = mcli->req_cli_param;
	memset(recv_param.cmd, 0x0, sizeof(recv_param.cmd));

	if (((mparam->flag & FLAG_SYNC_MUTEX) == FLAG_SYNC_MUTEX) && (mparam->mlock != NULL)) {
		plogd("[mutex] unlock prarent thread <%s:%d>", __FUNCTION__,__LINE__);
		pthread_mutex_unlock(mparam->mlock);
	}

	pthread_mutex_lock(&mcli->sync_lock);
	mcli->cli_recv_ready = 1;
	pthread_mutex_unlock(&mcli->sync_lock);

	// process recv async
	mcli->recv_message_async(&recv_param);

	close(mfd);

	if ((mparam->servcfg == NULL) && (mcli != NULL)) {
		delete(mcli);
		mcli = NULL;
	}

	return (void*)NULL;
}

// handle servcfg message action,
// new process/pthread from local service accept
void*
service_manager_serv_handler(void* param)
{
	int mfd;
	proc_conf   *mpconf = (proc_conf*)param;
	service_manager *mserv       = NULL;
	serv_conf      *mserv_conf = NULL;
	message_param   *msg_param   = NULL;

	if (mpconf == NULL) {
		ploge("%s param is null, abort!", __FUNCTION__);
		return NULL;
	}

	if (mpconf->servcfg != NULL) {
		mserv_conf = mpconf->servcfg;
		if (mserv_conf->serv_cls != NULL) {
			mserv = (service_manager*)mserv_conf->serv_cls;
		} else {
			mserv = new service_manager();
		}
	} else {
		mserv = new service_manager();
	}

	if (mpconf->param != NULL)
		msg_param = (message_param*)mpconf->param;
	else
		msg_param = new message_param();

	// mpconf will become null after mutex unlock
	mfd = dup(mpconf->listenfd);
	close(mpconf->listenfd);

	msg_param->sockfd = mfd;
	msg_param->param = NULL;
	memset(msg_param->cmd, 0x0, sizeof(msg_param->cmd));

	// Unlock mlock from parent thread creater process_manager
	if (mpconf->mlock != NULL)
		pthread_mutex_unlock(mpconf->mlock);

	plogd("[pipe]\tprocess ready and write pipe to notify parent.");
	write(mpconf->pipefd[1], "ready", strlen("ready"));
	close(mpconf->pipefd[0]);
	close(mpconf->pipefd[1]);

	// infinity loop to read/write data
	// if ((mpconf->flag & FLAG_MSG_ASYNC) == FLAG_MSG_ASYNC) {
		mserv->respond_cmd_async(mfd, NULL);
	// } else {
	//     mserv->respond_cmd(mfd, NULL);
	// }

	if (mpconf->servcfg != NULL) {
		if ((mserv_conf->serv_cls == NULL) && (mserv != NULL)) {
			delete(mserv);
			mserv = NULL;
		}
	} else if (mserv != NULL) {
		delete(mserv);
		mserv = NULL;
	}

	close(mfd);

	if (mpconf->param == NULL) {
		delete msg_param;
		msg_param = NULL;
	}
	plogd("debug %s:%d", __FILE__, __LINE__);
	return (void*)NULL;
}

void*
service_manager::manager_cli_handler(void* param)
{
	int mfd;
	proc_conf   *mproc_param     = (proc_conf*)param;
	service_manager *mserv           = NULL;
	serv_conf      *mserv_conf     = NULL;
	message_param   *msg_param       = NULL;

	if (mproc_param == NULL) {
		ploge("%s param is null, abort!", __FUNCTION__);
		return NULL;
	}

	if (mproc_param->servcfg != NULL) {
		mserv_conf = mproc_param->servcfg;
		if (mserv_conf->serv_cls != NULL) {
			mserv = (service_manager*)mserv_conf->serv_cls;
		} else {
			mserv = new service_manager();
		}
	} else {
		mserv = new service_manager();
	}

	mfd = dup(mproc_param->listenfd);

	// set message param
	if (mproc_param->param != NULL) {
		msg_param = (message_param*)mproc_param->param;
		plogd("get message from prarm, cmd: %s", msg_param->cmd);
	} else {
		plogd("new message");
		msg_param = new message_param();
	}
	msg_param->sockfd = mfd;
	msg_param->param = NULL;

	// Unlock mlock from parent thread creater process_manager
	if (mproc_param->mlock != NULL)
		pthread_mutex_unlock(mproc_param->mlock);

	// Start recv/send message thread
	mserv->start_client_manager_recv_pthread(msg_param);
		plogd("cmd: %s", msg_param->cmd);
	mserv->start_client_manager_send_pthread(msg_param);

	if (mproc_param->servcfg != NULL) {
		if ((mserv_conf->serv_cls == NULL) && (mserv != NULL)) {
			delete(mserv);
			mserv = NULL;
		}
	} else if (mserv != NULL) {
		delete(mserv);
		mserv = NULL;
	}

	close(mfd);
	close(mproc_param->listenfd);

	if ((mproc_param->param == NULL) && (msg_param != NULL)) {
		delete msg_param;
		msg_param = NULL;
	}
	plogd("debug %s:%d", __FILE__, __LINE__);
	return (void*)NULL;
}

int
service_manager::start_manager_server(void* param)
{
	proc_conf *mproc_param = (proc_conf*)param;
	plogd("Enter: %s:%d",__func__,__LINE__);

	if (param == NULL) {
		ploge("%s param is null, abort!", __FUNCTION__);
		return -EINVAL;
	}

	ml_serv.register_serv_handler(service_manager_serv_handler, NULL);
	ml_serv.start_socket_server(mproc_param);

	return 0;
}

int
service_manager::start_client_manager(void* param)
{
	proc_conf *mproc_param = (proc_conf*)param;
	plogd("Enter: %s:%d",__func__,__LINE__);

	if (param == NULL) {
		ploge("%s param is null, abort!", __FUNCTION__);
		return -EINVAL;
	}

	ml_serv.register_cli_handler(manager_cli_handler, mproc_param->param);
	ml_serv.start_local_client(mproc_param);

	return 0;
}

int
service_manager::start_client_manager_recv_pthread(message_param *msg_param)
{
	process_manager mproc_recv;
	proc_conf   mproc_recv_param;
	serv_conf      mserv_conf;

	plogd("byJames %s:%d", __FILE__,__LINE__);
	if (msg_param == NULL) {
		return -EINVAL;
	}

	mserv_conf.serv_cls = this;

	mproc_recv_param.listenfd = msg_param->sockfd;
	mproc_recv_param.mlock    = mproc_recv.get_thread_mutex();
	mproc_recv_param.flag     = FLAG_WITH_PTHREAD;
	mproc_recv_param.flag    |= FLAG_SYNC_MUTEX;
	mproc_recv_param.servcfg  = &mserv_conf;
	mproc_recv_param.param    = (void*)&msg_param;

	mproc_recv.start_routine = start_client_manager_recv_proc;
	mproc_recv.start_thread_sync_mutex((void*)&mproc_recv_param);
	return 0;
}

int
service_manager::start_client_manager_send_pthread(message_param *msg_param)
{
	process_manager mproc_send;
	proc_conf   mproc_send_param;
	serv_conf      mserv_conf;

	if (msg_param == NULL) {
		return -EINVAL;
	}

	mserv_conf.serv_cls = this;

	mproc_send_param.listenfd = msg_param->sockfd;
	mproc_send_param.mlock    = mproc_send.get_thread_mutex();
	mproc_send_param.flag    |= FLAG_WITH_PTHREAD;
	mproc_send_param.flag    |= FLAG_SYNC_MUTEX;
	mproc_send_param.flag    |= FLAG_BLOCK;
	mproc_send_param.servcfg  = &mserv_conf;
	mproc_send_param.param    = (void*)msg_param;

	mproc_send.start_routine = start_client_manager_proc;
	mproc_send.start_thread_sync_mutex((void*)&mproc_send_param);

	return 0;
}

void
service_manager::set_service_type(int type)
{
	service_type = type;
}

int
service_manager::respond_cmd_async(int sockfd, void* param)
{
	int msockfd = -1;
	int fd_event;
	int i0;
	int rlen;
	int wlen;
	char buffer[CONFIG_PROCMGR_BUFFER_LEN];
	int ret = 0;
	int fd_max;
	fd_set rd_set;
	int mlink;
	int sel_ret;
	int nl_groups;

	netlink_event mevent;

	nl_groups = RTMGRP_LINK;
	mevent.init_netlink_socket(nl_groups);
	fd_event = mevent.get_uevent_sockfd();

	msockfd = sockfd;
	fd_max = PROCMGR_MAX_VAL(msockfd, fd_event) + 1;

	plogd("fd_event: %d, msockfd: %d, fd_max: %d", fd_event, msockfd, fd_max);
	i0 = 0;
	while(true) {
		FD_ZERO(&rd_set);

		FD_SET(msockfd  , &rd_set);
		FD_SET(fd_event , &rd_set);

		// set timeval NULL to block
		sel_ret = select(fd_max, &rd_set, NULL, NULL , NULL);
		if (sel_ret < 0 ) {
			plogd();
			return -EINVAL;
		}

		if (FD_ISSET(msockfd, &rd_set))
		{
			message_param mserv_msg;

			mserv_msg.sockfd = sockfd;
			mserv_msg.param = param;
			memset(mserv_msg.cmd, 0x0, sizeof(mserv_msg.cmd));
			rlen = read(mserv_msg.sockfd, mserv_msg.cmd, sizeof(mserv_msg.cmd) - 1);
			plogd("[%s:%d] get msg<%d>, len:[%d] : %s", __func__,__LINE__,i0, rlen, mserv_msg.cmd);

			if(rlen <= 0 && sel_ret == 1) {
				ploge("[%d] closing socket %d!, rlen: %d, ret: %d", __LINE__, mserv_msg.sockfd, rlen, sel_ret);
				break;
			}

			mmsg.serv_proc_messages(&mserv_msg);

			wlen = write(mserv_msg.sockfd, mserv_msg.cmd, strlen(mserv_msg.cmd) + 1);
			plogd("[%s:%d] send msg<%d>, len:[%d] : %s", __func__, __LINE__, i0, wlen, mserv_msg.cmd);

			if(strcasecmp(mserv_msg.cmd, "quit QUIT") == 0) {
				plogd("quit this process");
				break;
			}
		}
		else if (FD_ISSET(fd_event, &rd_set))
		{
			memset(buffer,0x0,sizeof(buffer));
			mlink = mevent.read_event(msg_handler);

			wlen = write(msockfd, buffer, strlen(buffer) + 1);
			plogd("send msg %d, len:[%d] : %s", i0, wlen, buffer);
			continue;
		}


		if(rlen <= 0 && sel_ret == 1)
		{
			plogd("[%d] closing socket: %d!, rlen: %d, ret: %d",__LINE__, msockfd , rlen, ret);
			break;
		}
		i0++;
	}
	if (msockfd > 0) {
		plogd("closing %d", msockfd);
		// close(msockfd);
	}
	FD_ZERO(&rd_set);
	return 0;
}

int
service_manager::respond_cmd(int sockfd, void* param)
{
	int i0;
	int rlen;
	int wlen;

	i0 = 0;
	while(true) {
		message_param mserv_msg;

		mserv_msg.sockfd = sockfd;
		mserv_msg.param = param;
		memset(mserv_msg.cmd, 0x0, sizeof(mserv_msg.cmd));
		rlen = read(mserv_msg.sockfd, mserv_msg.cmd, sizeof(mserv_msg.cmd) - 1);
		plogd("get msg<%d>, len:[%d] : %s", i0, rlen, mserv_msg.cmd);

		if(rlen <= 0) {
			ploge("socket closed!, rlen: %d", rlen);
			break;
		}

		mmsg.serv_proc_messages(&mserv_msg);

		wlen = write(mserv_msg.sockfd, mserv_msg.cmd, strlen(mserv_msg.cmd) + 1);
		plogd("%s:%d send msg<%d>, len:[%d] : %s", __func__, __LINE__, i0, wlen, mserv_msg.cmd);

		if(strcasecmp(mserv_msg.cmd, "quit QUIT") == 0) {
			plogd("quit this process");
			break;
		}

		if (rlen <= 0 && wlen < 0) {
			break;
		}
		i0++;
	}

	return 0;
}

int
service_manager::recv_message_async(message_param *param)
{
	int msockfd = -1;
	int i0;
	int rlen;
	int wlen;
	char buffer[CONFIG_PROCMGR_BUFFER_LEN];
	int ret;
	int fd_max;
	fd_set rd_set;

	message_param *msg = param;
	message_param *msg_ret = NULL;

	if (msg == NULL) {
		ploge("[%s:%d] param is NULL, abort!", __FILE__,__LINE__);
		return -EINVAL;
	}
	msockfd = msg->sockfd;
	fd_max = PROCMGR_MAX_VAL(msockfd, STDIN_FILENO)  + 1;

	i0 = 0;
	while(true) {
		FD_ZERO(&rd_set);

		FD_SET(msockfd      , &rd_set);
		FD_SET(STDIN_FILENO , &rd_set);

		// set timeval NULL to block
		ret = select(fd_max, &rd_set, NULL, NULL , NULL);
		if (ret < 0 ) {
			plogd("fd select incorrect, abort!");
			return -EINVAL;
		}

		if (FD_ISSET(msockfd, &rd_set))
		{
			memset(buffer,0x0,sizeof(buffer));
			rlen = read(msockfd, buffer, sizeof(buffer) - 1);
			plogd("async get msg %d, len:[%d] : %s", i0, rlen, buffer);
			msg_ret = (message_param*)mmsg.cli_proc_messages(msockfd, buffer,NULL);
			if(msg_ret == NULL)
			{
				plogd("%s ret null; break", __FUNCTION__);
				break;
			}
			else
			{
				// plogd("servcfg cmd: %s", msg_ret->cmd);
				if (strcasecmp(msg_ret->cmd, "quit") == 0) {
					plogd("%s get msg: %s, abort", __func__, msg_ret->cmd);
					break;
				// } else if (msg_ret.param == NULL) {
				//     plogd("%s get null param, abort")
				//     break;
				// } else {
				//     break;
				}
			}

			if(rlen <= 0 && ret == 1)
			{
				plogd("closing sockfd: %d!, rlen: %d, ret: %d", msockfd, rlen, ret);
				break;
			}
		}

		i0++;
	}
	close(msockfd);
	return 0;
}

int
service_manager::send_request_cmd(message_param *param)
{
	int msockfd = -1;
	int i0;
	int rlen;
	int wlen;
	char buffer[CONFIG_PROCMGR_BUFFER_LEN];
	int ret;
	int fd_max;
	fd_set rd_set;
	message_param *msg_param = param;

	msockfd = msg_param->sockfd;
	// fd_max = PROCMGR_MAX_VAL(msockfd, STDIN_FILENO)  + 1;
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

int
service_manager::send_request_cmd_once(message_param *param)
{
	int msockfd = -1;
	int wlen;
	char buffer[CONFIG_PROCMGR_BUFFER_LEN];
	message_param *msg_param = param;

	msockfd = msg_param->sockfd;
	plogd("send message: %s", msg_param->cmd);
	wlen = write(msockfd, msg_param->cmd, strlen(msg_param->cmd)+1);
	close(msockfd);
	return 0;
}

int
service_manager::register_cli_send_msg_callback(void *(*proc_func)(void* param), void *param)
{
	cli_send_msg_callback = proc_func;
	req_cli_param = param;
	return 0;
}

int
service_manager::register_serv_msg_callback(void *(*proc_func)(void* param), void *param)
{
	mmsg.register_serv_proc_callback(proc_func, param);
	return 0;
}

int
service_manager::register_cli_recv_msg_callback(void *(*proc_func)(void* param), void *param)
{
	mmsg.register_cli_proc_callback(proc_func, param);
	return 0;
}

int
service_manager::msg_handler(struct sockaddr_nl *nl, struct nlmsghdr *msg)
{
    struct ifinfomsg *ifi=(struct ifinfomsg*)NLMSG_DATA(msg);
    struct ifaddrmsg *ifa=(struct ifaddrmsg*)NLMSG_DATA(msg);
    char ifname[CONFIG_PROCMGR_MAXLEN];
	int ret = 0;
	if_indextoname(ifa->ifa_index,ifname);

    switch (msg->nlmsg_type)
    {
        case RTM_NEWLINK:
            plogd("%s: %s RTM_NEWLINK\n", __FUNCTION__, ifname);
            break;
		case RTM_DELLINK:
            plogd("%s: %s RTM_DELLINK\n", __FUNCTION__, ifname);
			break;
		case RTM_NEWADDR:
            plogd("%s: %s RTM_NEWADDR\n", __FUNCTION__, ifname);
			break;
		case RTM_DELADDR:
            plogd("%s: %s RTM_DELADDR\n", __FUNCTION__, ifname);
			break;
		case RTM_NEWROUTE:
            plogd("%s: %s RTM_NEWROUTE\n", __FUNCTION__, ifname);
			break;
		case RTM_DELROUTE:
            plogd("%s: %s RTM_DELROUTE\n", __FUNCTION__, ifname);
			break;
		case RTM_GETROUTE:
            plogd("%s: %s RTM_GETROUTE\n", __FUNCTION__, ifname);
			break;
        default:
            plogd("%s: %s Unknown netlink nlmsg_type %\n", __FUNCTION__,
                    ifname, msg->nlmsg_type);
            break;
    }
    return ret;
}

