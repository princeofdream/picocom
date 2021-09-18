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
 *         Author:  Li Jin, lij1@xiaopeng.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#include <service_manager.h>
#include <netlink_event.h>


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
start_service_manager_proc(void* param)
{
	int mfd;

	static netlink_event mevent;
	service_manager      *srvmgr = NULL;
	process_param        *mparam = (process_param*)param;
	process_param        mproc_param;
	serv_param           *mserv_param = NULL;

	if (mparam == NULL) {
		loge("%s param is null, abort!", __FUNCTION__);
		return (void*)NULL;
	}

	if (mparam->serv != NULL) {
		mserv_param = mparam->serv;
		if (mserv_param->serv_cls != NULL)
			srvmgr = (service_manager*)mparam->serv->serv_cls;
		else
			srvmgr = new service_manager();
	} else {
		srvmgr = new service_manager();
	}

	// set process_manager mlock to child thread
	if (mserv_param != NULL)
		mserv_param->serv_cls = srvmgr;

	mproc_param.mlock = mparam->mlock;
	mproc_param.flags  = mparam->flags;
	mproc_param.serv  = mserv_param;
	mproc_param.param = mparam->param;

	// misc_utils mmisc;
	// mmisc.flag_to_string(mproc_param.flags, __func__);

	memset(mproc_param.cmd, 0x0, sizeof(mproc_param.cmd));
	sprintf(mproc_param.cmd, "%s", mparam->cmd);

	if (srvmgr->service_type == MGR_SERVICE_SERV)
		srvmgr->start_manager_server(&mproc_param);
	else if (srvmgr->service_type == MGR_SERVICE_CLI)
		srvmgr->start_manager_client(&mproc_param);
	else
		log("service type incorrect, Do not start manager service.\n");

	logd("==== service done, quit... ====");
	if((mparam->serv != NULL) && (mserv_param->serv_cls == NULL) && (srvmgr != NULL)) {
		delete(srvmgr);
		srvmgr = NULL;
	} else if((mparam->serv == NULL) && (srvmgr != NULL)) {
		delete(srvmgr);
		srvmgr = NULL;
	}
	return (void*)NULL;
}

void*
start_manager_client_proc(void* param)
{
	int mfd = -1;
	netlink_event mevent;
	process_param *mparam = (process_param*)param;
	service_manager *mcli = NULL;
	message_param *req_msg_param;

	if (mparam == NULL) {
		loge("%s param is null, abort!", __FUNCTION__);
		return (void*)NULL;
	}

	if (mparam->serv != NULL) {
		mcli = (service_manager*)mparam->serv->serv_cls;
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

	if (((mparam->flags & FLAG_SYNC_MUTEX) == FLAG_SYNC_MUTEX) && (mparam->mlock != NULL)) {
		logd("[mutex] unlock prarent thread <%s:%d>", __FUNCTION__,__LINE__);
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

	if ((mparam->serv == NULL) && (mcli != NULL)) {
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
start_manager_client_recv_proc(void* param)
{
	int mfd = -1;
	netlink_event mevent;
	process_param *mparam = (process_param*)param;
	service_manager *mcli = NULL;
	message_param recv_param;

	if (mparam == NULL) {
		loge("%s param is null, abort!", __FUNCTION__);
		return (void*)NULL;
	}

	if (mparam->serv != NULL) {
		mcli = (service_manager*)mparam->serv->serv_cls;
	} else {
		mcli = new service_manager();
	}

	mfd = dup(mparam->listenfd);

	recv_param.sockfd = mfd;
	recv_param.param = mcli->req_cli_param;
	memset(recv_param.cmd, 0x0, sizeof(recv_param.cmd));

	if (((mparam->flags & FLAG_SYNC_MUTEX) == FLAG_SYNC_MUTEX) && (mparam->mlock != NULL)) {
		logd("[mutex] unlock prarent thread <%s:%d>", __FUNCTION__,__LINE__);
		pthread_mutex_unlock(mparam->mlock);
	}

	pthread_mutex_lock(&mcli->sync_lock);
	mcli->cli_recv_ready = 1;
	pthread_mutex_unlock(&mcli->sync_lock);

	// process recv async
	mcli->recv_message_async(&recv_param);

	close(mfd);

	if ((mparam->serv == NULL) && (mcli != NULL)) {
		delete(mcli);
		mcli = NULL;
	}

	return (void*)NULL;
}

// handle serv message action,
// new process/pthread from local service accept
void*
service_manager_serv_handler(void* param)
{
	int mfd;
	process_param   *mproc_param = (process_param*)param;
	service_manager *mserv       = NULL;
	serv_param      *mserv_param = NULL;
	message_param   *msg_param   = NULL;

	if (mproc_param == NULL) {
		loge("%s param is null, abort!", __FUNCTION__);
		return NULL;
	}

	if (mproc_param->serv != NULL) {
		mserv_param = mproc_param->serv;
		if (mserv_param->serv_cls != NULL) {
			mserv = (service_manager*)mserv_param->serv_cls;
		} else {
			mserv = new service_manager();
		}
	} else {
		mserv = new service_manager();
	}

	if (mproc_param->param != NULL)
		msg_param = (message_param*)mproc_param->param;
	else
		msg_param = new message_param();

	// mproc_param will become null after mutex unlock
	mfd = dup(mproc_param->listenfd);
	close(mproc_param->listenfd);

	msg_param->sockfd = mfd;
	msg_param->param = NULL;
	memset(msg_param->cmd, 0x0, sizeof(msg_param->cmd));

	// Unlock mlock from parent thread creater process_manager
	if (mproc_param->mlock != NULL)
		pthread_mutex_unlock(mproc_param->mlock);

	logd("[pipe]\tprocess ready and write pipe to notify parent.");
	write(mproc_param->pipefd[1], "ready", strlen("ready"));

	// infinity loop to read/write data
	// if ((mproc_param->flags & FLAG_MSG_ASYNC) == FLAG_MSG_ASYNC) {
		mserv->respond_cmd_async(mfd, NULL);
	// } else {
	//     mserv->respond_cmd(mfd, NULL);
	// }

	if (mproc_param->serv != NULL) {
		if ((mserv_param->serv_cls == NULL) && (mserv != NULL)) {
			delete(mserv);
			mserv = NULL;
		}
	} else if (mserv != NULL) {
		delete(mserv);
		mserv = NULL;
	}

	close(mfd);

	if (mproc_param->param == NULL) {
		delete msg_param;
		msg_param = NULL;
	}
	logd("debug %s:%d", __FILE__, __LINE__);
	return (void*)NULL;
}

void*
service_manager_cli_handler(void* param)
{
	int mfd;
	process_param   *mproc_param = (process_param*)param;
	service_manager *srvmgr      = NULL;
	serv_param      *mserv_param = NULL;
	message_param   *msg_param   = NULL;

	if (mproc_param == NULL) {
		loge("%s param is null, abort!", __FUNCTION__);
		return NULL;
	}

#if 0
	char buf[1024];
	memset(buf,0x0,sizeof(buf));
	sprintf(buf, "%s","james_msg_debug");
	write(mproc_param->listenfd, buf, strlen(buf));
	read(mproc_param->listenfd, buf, sizeof(buf));
#endif
	if (mproc_param->serv != NULL) {
		mserv_param = mproc_param->serv;
		if (mserv_param->serv_cls != NULL) {
			srvmgr = (service_manager*)mserv_param->serv_cls;
		} else {
			srvmgr = new service_manager();
		}
	} else {
		srvmgr = new service_manager();
	}

	mfd = dup(mproc_param->listenfd);

	// set message param
	if (mproc_param->param != NULL) {
		msg_param = (message_param*)mproc_param->param;
		logd("get message from prarm, cmd: %s", msg_param->cmd);
	} else {
		logd("new message");
		msg_param = new message_param();
	}
	msg_param->sockfd = mfd;
	msg_param->param = NULL;

	// Unlock mlock from parent thread creater process_manager
	if (mproc_param->mlock != NULL)
		pthread_mutex_unlock(mproc_param->mlock);

	// Start recv/send message thread
	srvmgr->start_manager_client_recv_pthread(msg_param);
		logd("cmd: %s", msg_param->cmd);
	srvmgr->start_manager_client_send_pthread(msg_param);

	if (mproc_param->serv != NULL) {
		if ((mserv_param->serv_cls == NULL) && (srvmgr != NULL)) {
			delete(srvmgr);
			srvmgr = NULL;
		}
	} else if (srvmgr != NULL) {
		delete(srvmgr);
		srvmgr = NULL;
	}

	close(mfd);
	close(mproc_param->listenfd);

	if ((mproc_param->param == NULL) && (msg_param != NULL)) {
		delete msg_param;
		msg_param = NULL;
	}
	logd("debug %s:%d", __FILE__, __LINE__);
	return (void*)NULL;
}

int
service_manager::start_manager_server(void* param)
{
	process_param *mproc_param = (process_param*)param;
	local_service ml_serv;

	if (param == NULL) {
		loge("%s param is null, abort!", __FUNCTION__);
		return -EINVAL;
	}

	ml_serv.register_serv_handler(service_manager_serv_handler, param);
	ml_serv.start_local_service(mproc_param);

	return 0;
}

int
service_manager::start_manager_client(void* param)
{
	process_param *mproc_param = (process_param*)param;
	local_service ml_serv;

	if (param == NULL) {
		loge("%s param is null, abort!", __FUNCTION__);
		return -EINVAL;
	}

	if (mproc_param->param == NULL) {
		DBG("param is null");
	}

	if ((mproc_param->flags & FLAG_BLOCK) != FLAG_BLOCK) {
		mproc_param->flags |= FLAG_BLOCK;
	}
	ml_serv.register_cli_handler(service_manager_cli_handler, mproc_param->param);
	ml_serv.start_local_client(mproc_param);
	mproc_param->listenfd = ml_serv.get_client_fd();
	DBG("start_manager_client done");

	return 0;
}

int
service_manager::start_manager_client_recv_pthread(message_param *msg_param)
{
	process_manager mproc_recv;
	process_param   mproc_recv_param;
	serv_param      mserv_param;

	DBG("Enter %s", __FUNCTION__);
	if (msg_param == NULL) {
		return -EINVAL;
	}

	mserv_param.serv_cls = this;

	mproc_recv_param.listenfd = msg_param->sockfd;
	mproc_recv_param.mlock    = mproc_recv.get_thread_mutex();
	mproc_recv_param.flags     = FLAG_WITH_PTHREAD;
	mproc_recv_param.flags    |= FLAG_SYNC_MUTEX;
	mproc_recv_param.serv     = &mserv_param;
	mproc_recv_param.param    = (void*)&msg_param;

	mproc_recv.start_routine = start_manager_client_recv_proc;
	mproc_recv.start_thread_sync_mutex((void*)&mproc_recv_param);
	return 0;
}

int
service_manager::start_manager_client_send_pthread(message_param *msg_param)
{
	process_manager mproc_send;
	process_param   mproc_send_param;
	serv_param      mserv_param;

	if (msg_param == NULL) {
		return -EINVAL;
	}

	mserv_param.serv_cls = this;

	mproc_send_param.listenfd = msg_param->sockfd;
	mproc_send_param.mlock    = mproc_send.get_thread_mutex();
	mproc_send_param.flags    |= FLAG_WITH_PTHREAD;
	mproc_send_param.flags    |= FLAG_SYNC_MUTEX;
	mproc_send_param.flags    |= FLAG_BLOCK;
	mproc_send_param.serv     = &mserv_param;
	mproc_send_param.param    = (void*)msg_param;

	mproc_send.start_routine = start_manager_client_proc;
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
	char buffer[READ_BUFFER_LEN];
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
	fd_max = MAX_VAL(msockfd, fd_event) + 1;

	DBG("fd_event: %d, msockfd: %d, fd_max: %d", fd_event, msockfd, fd_max);
	i0 = 0;
	while(true) {
		FD_ZERO(&rd_set);

		FD_SET(msockfd  , &rd_set);
		FD_SET(fd_event , &rd_set);

		// set timeval NULL to block
		sel_ret = select(fd_max, &rd_set, NULL, NULL , NULL);
		if (sel_ret < 0 ) {
			return -EINVAL;
		}

		if (FD_ISSET(msockfd, &rd_set))
		{
			message_param mserv_msg;

			mserv_msg.sockfd = sockfd;
			mserv_msg.param = param;
			memset(mserv_msg.cmd, 0x0, sizeof(mserv_msg.cmd));
			rlen = read(mserv_msg.sockfd, mserv_msg.cmd, sizeof(mserv_msg.cmd) - 1);
			logd("[%s:%d] get msg<%d>, len:[%d] : %s", __func__,__LINE__,i0, rlen, mserv_msg.cmd);

			if(rlen <= 0 && sel_ret == 1) {
				loge("socket closed!, rlen: %d, ret: %d", rlen, sel_ret);
				break;
			}

			mmsg.serv_proc_messages(&mserv_msg);

			wlen = write(mserv_msg.sockfd, mserv_msg.cmd, strlen(mserv_msg.cmd) + 1);
			logd("[%s:%d] send msg<%d>, len:[%d] : %s", __func__, __LINE__, i0, wlen, mserv_msg.cmd);

			if(strcasecmp(mserv_msg.cmd, "quit QUIT") == 0) {
				logd("quit this process");
				break;
			}
		}
		else if (FD_ISSET(fd_event, &rd_set))
		{
			memset(buffer,0x0,sizeof(buffer));
			mlink = mevent.read_event(msg_handler);

			wlen = write(msockfd, buffer, strlen(buffer) + 1);
			DBG("send msg %d, len:[%d] : %s", i0, wlen, buffer);
			continue;
		}


		if(rlen <= 0 && sel_ret == 1)
		{
			DBG("socket closed!, rlen: %d, ret: %d", rlen, ret);
			break;
		}
		i0++;
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
		logd("get msg<%d>, len:[%d] : %s", i0, rlen, mserv_msg.cmd);

		if(rlen <= 0) {
			loge("socket closed!, rlen: %d", rlen);
			break;
		}

		mmsg.serv_proc_messages(&mserv_msg);

		wlen = write(mserv_msg.sockfd, mserv_msg.cmd, strlen(mserv_msg.cmd) + 1);
		logd("%s:%d send msg<%d>, len:[%d] : %s", __func__, __LINE__, i0, wlen, mserv_msg.cmd);

		if(strcasecmp(mserv_msg.cmd, "quit QUIT") == 0) {
			logd("quit this process");
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
	char buffer[READ_BUFFER_LEN];
	int ret;
	int fd_max;
	fd_set rd_set;

	message_param *msg = param;
	message_param *msg_ret = NULL;

	if (msg == NULL) {
		loge("[%s:%d] param is NULL, abort!", __FILE__,__LINE__);
		return -EINVAL;
	}
	msockfd = msg->sockfd;
	fd_max = MAX_VAL(msockfd, STDIN_FILENO)  + 1;

	i0 = 0;
	while(true) {
		FD_ZERO(&rd_set);

		FD_SET(msockfd      , &rd_set);
		FD_SET(STDIN_FILENO , &rd_set);

		// set timeval NULL to block
		ret = select(fd_max, &rd_set, NULL, NULL , NULL);
		if (ret < 0 ) {
			logd("fd select incorrect, abort!");
			return -EINVAL;
		}

		if (FD_ISSET(msockfd, &rd_set))
		{
			memset(buffer,0x0,sizeof(buffer));
			rlen = read(msockfd, buffer, sizeof(buffer) - 1);
			logd("async get msg %d, len:[%d] : %s", i0, rlen, buffer);
			msg_ret = (message_param*)mmsg.cli_proc_messages(msockfd, buffer,NULL);
			if(msg_ret == NULL)
			{
				logd("%s ret null; break", __FUNCTION__);
				break;
			}
			else
			{
				// logd("serv cmd: %s", msg_ret->cmd);
				if (strcasecmp(msg_ret->cmd, "quit") == 0) {
					logd("%s get msg: %s, abort", __func__, msg_ret->cmd);
					break;
				// } else if (msg_ret.param == NULL) {
				//     logd("%s get null param, abort")
				//     break;
				// } else {
				//     break;
				}
			}

			if(rlen <= 0 && ret == 1)
			{
				logd("socket closed!, rlen: %d, ret: %d", rlen, ret);
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

int
service_manager::send_request_cmd_once(message_param *param)
{
	int msockfd = -1;
	int wlen;
	char buffer[READ_BUFFER_LEN];
	message_param *msg_param = param;

	msockfd = msg_param->sockfd;
	logd("send message: %s", msg_param->cmd);
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
    char ifname[MAXLEN];
	int ret = 0;
	if_indextoname(ifa->ifa_index,ifname);

    switch (msg->nlmsg_type)
    {
        case RTM_NEWLINK:
            logd("%s: %s RTM_NEWLINK\n", __FUNCTION__, ifname);
            break;
		case RTM_DELLINK:
            logd("%s: %s RTM_DELLINK\n", __FUNCTION__, ifname);
			break;
		case RTM_NEWADDR:
            logd("%s: %s RTM_NEWADDR\n", __FUNCTION__, ifname);
			break;
		case RTM_DELADDR:
            logd("%s: %s RTM_DELADDR\n", __FUNCTION__, ifname);
			break;
		case RTM_NEWROUTE:
            logd("%s: %s RTM_NEWROUTE\n", __FUNCTION__, ifname);
			break;
		case RTM_DELROUTE:
            logd("%s: %s RTM_DELROUTE\n", __FUNCTION__, ifname);
			break;
		case RTM_GETROUTE:
            logd("%s: %s RTM_GETROUTE\n", __FUNCTION__, ifname);
			break;
        default:
            logd("%s: %s Unknown netlink nlmsg_type %\n", __FUNCTION__,
                    ifname, msg->nlmsg_type);
            break;
    }
    return ret;
}

