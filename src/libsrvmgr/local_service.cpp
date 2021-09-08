/*
 * =====================================================================================
 *
 *       Filename:  local_service.cpp
 *
 *    Description:  local_service
 *
 *        Version:  1.0
 *        Created:  03/07/2019 01:46:33 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, lij1@xiaopeng.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */

#include <local_service.h>
#include <manager_service.h>
#include <messages_manager.h>
#include <network_manager.h>

pthread_mutex_t mtime_lock;

void*
serv_handler(void* param)
{
	process_param *mparam = (process_param*)param;

	if (mparam == NULL)
		return NULL;

	logd("[%s:%d]--<%s>", __FILE__, __LINE__, __func__);

	if (mparam->mlock != NULL)
		pthread_mutex_unlock(mparam->mlock);
	return (void*)NULL;
}

void*
cli_handler(void* param)
{
	int msockfd = -1;
	int wlen;
	char buffer[READ_BUFFER_LEN];
	process_param *mparam = (process_param*)param;

	if (param == NULL)
		return NULL;

	if (mparam->mlock != NULL)
		pthread_mutex_unlock(mparam->mlock);

	msockfd = mparam->listenfd;
	logd("send message: %s", mparam->cmd);
	wlen = write(msockfd, mparam->cmd, strlen(mparam->cmd)+1);
	close(msockfd);

	return NULL;
}

local_service::local_service ()
{
	servfd   = -1;
	clientfd = -1;
	listenfd = -1;

	bzero(&servaddr   , sizeof(servaddr));
	bzero(&listenaddr , sizeof(listenaddr));
	bzero(&clientaddr , sizeof(clientaddr));

	pthread_mutex_init(&mtime_lock, NULL);

	msg_serv_callback     = NULL;
	cli_recv_msg_callback = NULL;
	cli_send_msg_callback = NULL;
	msg_serv_param        = NULL;
	msg_cli_param         = NULL;
	req_cli_param         = NULL;

	serv_handler_callback = NULL;
	serv_handler_param    = NULL;
	cli_handler_callback  = NULL;
	cli_handler_param     = NULL;

}

local_service::~local_service ()
{
	if (servfd > 0)
		close(servfd);

	if(clientfd > 0)
		close (clientfd);

	if(strcmp(servaddr.sun_path, SOCKET_SERVER_NAME) == 0)
		unlink(SOCKET_SERVER_NAME);
}


int
local_service::start_local_service(void* param)
{
	process_param *mparam = (process_param*)param;
	misc_utils mmsic;
	int ret;

	if (mparam == NULL) {
		loge("param is NULL, abort!");
		return -EINVAL;
	}

	// mmsic.flag_to_string(mparam->flag, __FUNCTION__);

	if ((mparam->flag & FLAG_WITH_IP) == FLAG_WITH_IP)
		setup_ip_server(mparam);
	else
		setup_server();

	// Unlock mlock from parent thread creater process_manager
	if (((mparam->flag & FLAG_SYNC_MUTEX) == FLAG_SYNC_MUTEX) && (mparam->mlock != NULL)) {
		logd("[mutex] unlock prarent thread <%s:%d>", __FUNCTION__,__LINE__);
		pthread_mutex_unlock(mparam->mlock);
	}

	ret = start_server(mparam);

	return 0;
}

int
local_service::setup_ip_server(void* param)
{
	int ret;
	int port;

	serv_param *mserv_param;
	process_param *mparam = (process_param*)param;

	if (mparam == NULL) {
		loge("param is NULL, abort!");
		return -EINVAL;
	}

	port = 8000;

	mserv_param = (serv_param*)mparam->serv;
	if (mserv_param != NULL) {
		if (mserv_param->port > 0)
			port = mserv_param->port;
	}

	memset(&serv_ip_addr,0,sizeof(serv_ip_addr));
	serv_ip_addr.sin_family=AF_INET;
	serv_ip_addr.sin_addr.s_addr=INADDR_ANY;
	serv_ip_addr.sin_port=htons(port);

	logd("Server Listen *:%d", port);

	//setup server socket
	servfd = socket(PF_INET,SOCK_STREAM, 0);
	if( servfd < 0 ) {
		loge("open socket error");
		return -EINVAL;
	}

	ret = bind(servfd, (struct sockaddr *)&serv_ip_addr, sizeof(struct sockaddr));
	if ( ret < 0) {
		loge("bind error");
		return -EINVAL;
	}

	listenlen = sizeof(struct sockaddr_in);
	listen(servfd, 5);

	return 0;
}

int
local_service::setup_server(void)
{
	int ret;

	DBG("Enter: %s", __FUNCTION__);

	servpid = getpid();
	servfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if ( servfd < 0 ) {
		loge("Fail to create socket");
		exit(servfd);
	}

	//setup server socket

	unlink(SOCKET_SERVER_NAME);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strncpy(servaddr.sun_path, SOCKET_SERVER_NAME, sizeof(servaddr.sun_path) - 1);
	servlen = sizeof(servaddr);

	ret = bind(servfd, (struct sockaddr *)&servaddr, servlen);
	if (servfd < 0) {
		loge("bind error");
		exit(servfd);
	}

	listenlen = sizeof(listenaddr);
	getsockname(servfd, (struct sockaddr *)&listenaddr, &listenlen);
	listen(servfd, 5);
	chmod(servaddr.sun_path, 0666);

	return 0;
}

int
local_service::start_server(void* param)
{
	int i0 = 0;
	struct sigaction act;
	process_param *mparam = (process_param*)param;

	DBG("Enter: %s", __FUNCTION__);
	if (mparam == NULL) {
		loge("param is NULL, abort!");
		return -EINVAL;
	}

	if((mparam->flag & FLAG_WITH_PTHREAD) == FLAG_WITH_PTHREAD)
	{
		sa.sa_handler = SIG_IGN;
		sigaction( SIGPIPE, &sa, 0 );
	}

	signal(SIGCHLD, SIG_IGN);

	// Once client connect, if netlink changes,
	// will respond messages to client, or get message from client
	while (true) {
		process_param    mproc_serv_param;
		serv_param       mserv;
		process_manager  mpm;
		char pipe_buffer[1024];
		int ss_opt = 1;

		if ((mparam->flag & FLAG_WITH_IP) == FLAG_WITH_IP)
			listenfd = accept(servfd, (struct sockaddr *)&serv_ip_addr, (socklen_t *)&listenlen);
		else
			listenfd = accept(servfd, (struct sockaddr *)&servaddr, (socklen_t *)&listenlen);

		if (listenfd < 0) {
			loge("%s, Fail to accept", __FUNCTION__);
			perror("Fail to accept");
			close(servfd);
			return -EINVAL;
		}

		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &ss_opt, sizeof(ss_opt));

		mproc_serv_param.listenfd  = listenfd;
		mproc_serv_param.servfd    = servfd;
		mproc_serv_param.ppid      = servpid;
		mproc_serv_param.mlock     = mpm.get_thread_mutex();
		mproc_serv_param.param     = serv_handler_param;
		mproc_serv_param.serv      = mparam->serv;
		mproc_serv_param.flag      = FLAG_WITH_PROCESS;
		// mproc_serv_param.flag      = FLAG_WITH_PTHREAD;
		// mproc_serv_param.flag     |= FLAG_SYNC_MUTEX;
		// mproc_serv_param.flag     |= FLAG_BLOCK;
		// this should get after start porcess
		mproc_serv_param.pipefd[0] = -EINVAL;
		mproc_serv_param.pipefd[1] = -EINVAL;

		logd("param fd: %d", mproc_serv_param.listenfd);
		if (serv_handler_callback != NULL)
			mpm.start_routine = serv_handler_callback;
		else
			mpm.start_routine = serv_handler;

		if ((mproc_serv_param.flag & FLAG_WITH_PTHREAD) == FLAG_WITH_PTHREAD) {
			mpm.start_thread_sync_mutex((void*)&mproc_serv_param);
		} else {
			mproc_serv_param.mlock = NULL;
			mpm.start_process((void*)&mproc_serv_param);
			//has close the connect in process, this parent process do not nedd the connection
			mproc_serv_param.pipefd[0] = mpm.get_pipe_fd_in();
			mproc_serv_param.pipefd[1] = mpm.get_pipe_fd_out();
			close(listenfd);

			memset(pipe_buffer, 0x0, sizeof(pipe_buffer));
			read(mproc_serv_param.pipefd[0], pipe_buffer, sizeof(pipe_buffer));
			logd("[pipe]\tpipe read: %s", pipe_buffer);
		}

		i0++;
	}

	DBG("End of service");

	if((mparam->flag & FLAG_WITH_PTHREAD) == FLAG_WITH_PTHREAD)
	{
		sigemptyset(&act.sa_mask);
		sigaddset(&act.sa_mask, SIGPIPE);
		act.sa_flags = 0;
		sigaction(SIGPIPE, &act, 0);
	}

	return 0;
}

int
local_service::start_local_client(void* param)
{
	process_param *mparam = (process_param*)param;

	if (mparam == NULL) {
		loge("param is NULL, abort!");
		return -EINVAL;
	}

	if ( (mparam->flag & FLAG_WITH_IP) == FLAG_WITH_IP)
		setup_ip_client(mparam);
	else
		setup_client();

	start_client(mparam);

	// Unlock mlock from parent thread creater process_manager
	if (((mparam->flag & FLAG_SYNC_MUTEX) == FLAG_SYNC_MUTEX) && (mparam->mlock != NULL)) {
		logd("[mutex] unlock prarent thread <%s:%d>", __FUNCTION__,__LINE__);
		pthread_mutex_unlock(mparam->mlock);
	}

	return 0;
}

int
local_service::setup_ip_client(void* param)
{
	int len;
	int ret;
	char ip_str[MAXLEN];
	int port;

	serv_param *mserv_param;
	process_param *mparam = (process_param*)param;

	if (mparam == NULL) {
		loge("param is NULL, abort!");
		return -EINVAL;
	}
	mserv_param = (serv_param*)mparam->serv;

	memset(ip_str, 0x0,sizeof(ip_str));
	sprintf(ip_str, "%s", "127.0.0.1");
	port = 8000;

	if (mserv_param != NULL) {
		if (strlen(mserv_param->ip_str) > 0)
			sprintf(ip_str, "%s", mserv_param->ip_str);
		if (mserv_param->port > 0)
			port = mserv_param->port;
	}

	DBG("Client connect ---> %s:%d <---", ip_str, port);

	memset(&client_ip_addr,0,sizeof(client_ip_addr));
	client_ip_addr.sin_family=AF_INET;
	client_ip_addr.sin_addr.s_addr=inet_addr(ip_str);
	client_ip_addr.sin_port=htons(port);


	clientfd=socket(PF_INET,SOCK_STREAM,0);
	if (clientfd < 0) {
		loge("fail to open socket");
		exit(EXIT_FAILURE);
	}



	DBG("connect to server: %s", "");
	ret = connect(clientfd,(struct sockaddr *)&client_ip_addr,sizeof(struct sockaddr));
	if (ret < 0) {
		loge ("fail to connect");
		exit (EXIT_FAILURE);
	}

	return 0;
}

int
local_service::setup_client(void)
{
	int len;
	int ret;

	if ((clientfd = socket(AF_LOCAL, SOCK_STREAM, 0)) == -1) {
		loge("fail to open socket");
		exit(EXIT_FAILURE);
	}

	clipid = getpid();

	clientaddr.sun_family = AF_UNIX;
	strncpy(clientaddr.sun_path, SOCKET_SERVER_NAME, sizeof(clientaddr.sun_path)-1);
	len = sizeof(clientaddr);

	DBG("Client connect ---> %s <---", clientaddr.sun_path);
	ret = connect (clientfd, (struct sockaddr *)&clientaddr, len);
	if (ret < 0)
	{
		loge ("fail to connect");
		exit (EXIT_FAILURE);
	}

	return 0;
}

int
local_service::start_client(void* param)
{
	char ch_recv[MAXLEN];
	char ch_send[MAXLEN];
	int bytes;
	int i0 = 0;
	process_param *mparam = (process_param*)param;
	// process_param mcli_send_param;
	// process_param mcli_recv_param;
	// process_manager mpm_send;
	// process_manager mpm_recv;
	process_param mproc_cli_param;
	process_manager mpm;
	serv_param mserv;

	logd("Enter: %s", __FUNCTION__);
	if (mparam == NULL) {
		loge("param is NULL, abort!");
		return -EINVAL;
	}

	// for client param
		mproc_cli_param.listenfd = clientfd;
		mproc_cli_param.ppid     = clipid;
		mproc_cli_param.mlock    = mpm.get_thread_mutex();
		mproc_cli_param.flag     = mparam->flag; // | FLAG_BLOCK;
		mproc_cli_param.param    = cli_handler_param;
		mproc_cli_param.serv     = mparam->serv;
		memset(mproc_cli_param.cmd, 0x0, sizeof(mproc_cli_param.cmd));
		sprintf(mproc_cli_param.cmd, "%s", mparam->cmd);

		pipe(mproc_cli_param.pipefd);

		if (cli_handler_callback != NULL)
			mpm.start_routine = cli_handler_callback;
		else
			mpm.start_routine = cli_handler;

		if ((mproc_cli_param.flag & FLAG_WITH_PTHREAD) == FLAG_WITH_PTHREAD) {
			mpm.start_thread_sync_mutex((void*)&mproc_cli_param);
		} else {
			mproc_cli_param.mlock = NULL;
			mpm.start_process((void*)&cli_handler_param);
			//has close the connect in process, this parent process do not nedd the connection
			close(listenfd);
		}

	return 0;
}

int
local_service::stop_local_service(void)
{
	return 0;
}

int
local_service::stop_local_client(void)
{
	return 0;
}

int
local_service::get_serv_fd(void)
{
	return servfd;
}

int
local_service::get_listen_fd(void)
{
	return listenfd;
}

int
local_service::get_client_fd(void)
{
	return clientfd;
}

int
local_service::register_serv_msg_callback(void *(*proc_func)(void* param), void *param)
{
	msg_serv_callback = proc_func;
	msg_serv_param = param;
	return 0;
}

int
local_service::register_cli_recv_msg_callback(void *(*proc_func)(void* param), void* param)
{
	cli_recv_msg_callback = proc_func;
	msg_cli_param = param;
	return 0;
}

int
local_service::register_cli_send_msg_callback(void *(*proc_func)(void* param), void* param)
{
	cli_send_msg_callback = proc_func;
	req_cli_param = param;
	return 0;
}


int
local_service::register_serv_handler(void *(*proc_func)(void* param), void* param)
{
	serv_handler_callback = proc_func;
	serv_handler_param = param;
	return NULL;
}

int
local_service::register_cli_handler(void *(*proc_func)(void* param), void* param)
{
	cli_handler_callback = proc_func;
	cli_handler_param = param;
	return NULL;
}

