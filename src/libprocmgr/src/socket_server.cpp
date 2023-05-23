/*
 * =====================================================================================
 *
 *       Filename:  socket_server.cpp
 *
 *    Description:  socket_server
 *
 *        Version:  1.0
 *        Created:  03/07/2019 01:46:33 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */

#include <socket_server.h>
#include <messages_manager.h>
#include <network_manager.h>

pthread_mutex_t mtime_lock;
static char default_serv_sock_path[1024];

socket_server_init::socket_server_init()
{
	plogd();
	memset(default_serv_sock_path, 0x0, sizeof(default_serv_sock_path));
#if defined(SERVER_SOCKET_PATH) && defined(SERVER_SOCKET_NAME)
	sprintf(default_serv_sock_path, "%s/%s", SERVER_SOCKET_PATH,SERVER_SOCKET_NAME);
#else
	sprintf(default_serv_sock_path, "%s", "/tmp/procmgr");
#endif
}

socket_server_init::socket_server_init(char* socket_path)
{
	plogd();
	memset(default_serv_sock_path, 0x0, sizeof(default_serv_sock_path));
	sprintf(default_serv_sock_path, "%s", socket_path);
}

socket_server_init::~socket_server_init()
{
}

void
socket_server_init::set_default_socket_server_path(char* socket_path)
{
	memset(default_serv_sock_path, 0x0, sizeof(default_serv_sock_path));
	sprintf(default_serv_sock_path, "%s", socket_path);
}

socket_server_init msocket_server_init;

void*
socket_server::serv_handler(void* param)
{
	proc_conf *mparam = (proc_conf*)param;

	if (mparam == NULL)
		return NULL;

	plogd("[%s:%d]--<%s>", __FILE__, __LINE__, __func__);

	if (mparam->mlock != NULL)
		pthread_mutex_unlock(mparam->mlock);
	return (void*)NULL;
}

void*
socket_server::cli_handler(void* param)
{
	int msockfd = -1;
	int wlen;
	char buffer[CONFIG_PROCMGR_BUFFER_LEN];
	proc_conf *mparam = (proc_conf*)param;

	if (param == NULL)
		return NULL;

	if (mparam->mlock != NULL)
		pthread_mutex_unlock(mparam->mlock);

	msockfd = mparam->listenfd;
	plogd("send message: %s", mparam->cmd);
	wlen = write(msockfd, mparam->cmd, strlen(mparam->cmd)+1);
	close(msockfd);

	return NULL;
}

socket_server::socket_server ()
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
	msg_serv_conf         = NULL;
	msg_cli_param         = NULL;
	req_cli_param         = NULL;

	serv_handler_callback = NULL;
	serv_handler_param    = NULL;
	cli_handler_callback  = NULL;
	cli_handler_param     = NULL;

	memset(serv_sock_path, 0x0, sizeof(serv_sock_path));
	if (strlen(default_serv_sock_path) > 0) {
		sprintf(serv_sock_path, "%s.%d.sock", default_serv_sock_path, getpid());
	} else {
		sprintf(serv_sock_path, "%s.%d.sock", "/tmp/procmgr", getpid());
	}
	plogd("serv_sock_path:%s", serv_sock_path);
}

socket_server::~socket_server ()
{
	plogd("..................");
	if (servfd > 0)
		close(servfd);

	if(clientfd > 0)
		close (clientfd);

	if(strcmp(servaddr.sun_path, serv_sock_path) == 0)
		unlink(serv_sock_path);
}


int
socket_server::start_socket_server(void* param)
{
	proc_conf *mparam = (proc_conf*)param;
	misc_utils mmsic;
	int ret;

	if (mparam == NULL) {
		ploge("param is NULL, abort!");
		return -EINVAL;
	}

	// mmsic.flag_to_string(mparam->flag, __FUNCTION__);

	if ((mparam->flag & FLAG_WITH_IP) == FLAG_WITH_IP)
		setup_ip_server(mparam);
	else
		setup_server();

	// Unlock mlock from parent thread creater process_manager
	if (((mparam->flag & FLAG_SYNC_MUTEX) == FLAG_SYNC_MUTEX) && (mparam->mlock != NULL)) {
		plogd("[mutex] unlock prarent thread <%s:%d>", __FUNCTION__,__LINE__);
		pthread_mutex_unlock(mparam->mlock);
	}

	ret = start_server(mparam);

	return 0;
}

int
socket_server::setup_ip_server(void* param)
{
	int ret;
	int port;

	serv_conf *mserv_conf;
	proc_conf *mparam = (proc_conf*)param;

	if (mparam == NULL) {
		ploge("param is NULL, abort!");
		return -EINVAL;
	}

	port = CONFIG_DEFAULT_SERV_PORT;

	mserv_conf = (serv_conf*)mparam->servcfg;
	if (mserv_conf != NULL) {
		if (mserv_conf->port > 0)
			port = mserv_conf->port;
	}

	memset(&serv_ip_addr,0,sizeof(serv_ip_addr));
	serv_ip_addr.sin_family=AF_INET;
	serv_ip_addr.sin_addr.s_addr=INADDR_ANY;
	serv_ip_addr.sin_port=htons(port);

	plogd("Server Listen *:%d", port);

	//setup server socket
	servfd = socket(PF_INET,SOCK_STREAM, 0);
	if( servfd < 0 ) {
		ploge("open socket error");
		return -EINVAL;
	}

	ret = bind(servfd, (struct sockaddr *)&serv_ip_addr, sizeof(struct sockaddr));
	if ( ret < 0) {
		ploge("bind error");
		return -EINVAL;
	}

	listenlen = sizeof(struct sockaddr_in);
	listen(servfd, 5);

	return 0;
}

int
socket_server::setup_server(void)
{
	int ret;

	pdbg("Enter: %s", __FUNCTION__);

	servpid = getpid();
	servfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if ( servfd < 0 ) {
		ploge("Fail to create socket");
		exit(servfd);
	}

	//setup server socket

	unlink(serv_sock_path);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strncpy(servaddr.sun_path, serv_sock_path, sizeof(servaddr.sun_path) - 1);
	servlen = sizeof(servaddr);

	ret = bind(servfd, (struct sockaddr *)&servaddr, servlen);
	if (servfd < 0) {
		ploge("bind error");
		exit(servfd);
	}

	listenlen = sizeof(listenaddr);
	getsockname(servfd, (struct sockaddr *)&listenaddr, &listenlen);
	listen(servfd, 5);
	chmod(servaddr.sun_path, 0666);

	return 0;
}

int
socket_server::start_server(void* param)
{
	int i0 = 0;
	struct sigaction act;
	proc_conf *mparam = (proc_conf*)param;

	pdbg("Enter: %s", __FUNCTION__);
	if (mparam == NULL) {
		ploge("param is NULL, abort!");
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
		proc_conf    mproc_serv_conf;
		serv_conf       mserv;
		process_manager  mpm;
		char pipe_buffer[1024];
		int ss_opt = 1;

		if ((mparam->flag & FLAG_WITH_IP) == FLAG_WITH_IP)
			listenfd = accept(servfd, (struct sockaddr *)&serv_ip_addr, (socklen_t *)&listenlen);
		else
			listenfd = accept(servfd, (struct sockaddr *)&servaddr, (socklen_t *)&listenlen);

		if (listenfd < 0) {
			ploge("%s, Fail to accept", __FUNCTION__);
			perror("Fail to accept");
			close(servfd);
			return -EINVAL;
		}

		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &ss_opt, sizeof(ss_opt));

		mproc_serv_conf.listenfd  = listenfd;
		mproc_serv_conf.servfd    = servfd;
		mproc_serv_conf.ppid      = servpid;
		mproc_serv_conf.mlock     = mpm.get_thread_mutex();
		mproc_serv_conf.param     = serv_handler_param;
		mproc_serv_conf.servcfg      = mparam->servcfg;
		mproc_serv_conf.flag      = FLAG_WITH_PROCESS;
		// mproc_serv_conf.flag      = FLAG_WITH_PTHREAD;
		// mproc_serv_conf.flag     |= FLAG_SYNC_MUTEX;
		// mproc_serv_conf.flag     |= FLAG_BLOCK;
		// this should get after start porcess
		mproc_serv_conf.pipefd[0] = -EINVAL;
		mproc_serv_conf.pipefd[1] = -EINVAL;

		plogd("param fd: %d", mproc_serv_conf.listenfd);
		if (serv_handler_callback != NULL)
			mpm.start_routine = serv_handler_callback;
		else
			mpm.start_routine = serv_handler;

		if ((mproc_serv_conf.flag & FLAG_WITH_PTHREAD) == FLAG_WITH_PTHREAD) {
			mpm.start_thread_sync_mutex((void*)&mproc_serv_conf);
		} else {
			mproc_serv_conf.mlock = NULL;
			mpm.start_process((void*)&mproc_serv_conf);
			//has close the connect in process, this parent process do not nedd the connection
			mproc_serv_conf.pipefd[0] = mpm.get_pipe_fd_in();
			mproc_serv_conf.pipefd[1] = mpm.get_pipe_fd_out();
			close(listenfd);

			memset(pipe_buffer, 0x0, sizeof(pipe_buffer));
			read(mproc_serv_conf.pipefd[0], pipe_buffer, sizeof(pipe_buffer));
			plogd("[pipe]\tpipe read: %s", pipe_buffer);
			close(mproc_serv_conf.pipefd[0]);
			close(mproc_serv_conf.pipefd[1]);
		}

		i0++;
	}

	pdbg("End of service");

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
socket_server::start_local_client(void* param)
{
	proc_conf *mparam = (proc_conf*)param;

	if (mparam == NULL) {
		ploge("param is NULL, abort!");
		return -EINVAL;
	}

	if ( (mparam->flag & FLAG_WITH_IP) == FLAG_WITH_IP)
		setup_ip_client(mparam);
	else
		setup_client();

	start_client(mparam);

	// Unlock mlock from parent thread creater process_manager
	if (((mparam->flag & FLAG_SYNC_MUTEX) == FLAG_SYNC_MUTEX) && (mparam->mlock != NULL)) {
		plogd("[mutex] unlock prarent thread <%s:%d>", __FUNCTION__,__LINE__);
		pthread_mutex_unlock(mparam->mlock);
	}

	return 0;
}

int
socket_server::setup_ip_client(void* param)
{
	int len;
	int ret;
	char ipaddr[CONFIG_PROCMGR_MAXLEN];
	int port;

	serv_conf *mserv_conf;
	proc_conf *mparam = (proc_conf*)param;

	if (mparam == NULL) {
		ploge("param is NULL, abort!");
		return -EINVAL;
	}
	mserv_conf = (serv_conf*)mparam->servcfg;

	memset(ipaddr, 0x0,sizeof(ipaddr));
	sprintf(ipaddr, "%s", "127.0.0.1");
	port = 8000;

	if (mserv_conf != NULL) {
		if (strlen(mserv_conf->ipaddr) > 0)
			sprintf(ipaddr, "%s", mserv_conf->ipaddr);
		if (mserv_conf->port > 0)
			port = mserv_conf->port;
	}

	pdbg("Client connect ---> %s:%d <---", ipaddr, port);

	memset(&client_ip_addr,0,sizeof(client_ip_addr));
	client_ip_addr.sin_family=AF_INET;
	client_ip_addr.sin_addr.s_addr=inet_addr(ipaddr);
	client_ip_addr.sin_port=htons(port);


	clientfd=socket(PF_INET,SOCK_STREAM,0);
	if (clientfd < 0) {
		ploge("fail to open socket");
		exit(EXIT_FAILURE);
	}



	pdbg("connect to server: %s", "");
	ret = connect(clientfd,(struct sockaddr *)&client_ip_addr,sizeof(struct sockaddr));
	if (ret < 0) {
		ploge("fail to connect");
		exit (EXIT_FAILURE);
	}

	return 0;
}

int
socket_server::setup_client(void)
{
	int len;
	int ret;

	if ((clientfd = socket(AF_LOCAL, SOCK_STREAM, 0)) == -1) {
		ploge("fail to open socket");
		exit(EXIT_FAILURE);
	}

	clipid = getpid();

	clientaddr.sun_family = AF_UNIX;
	strncpy(clientaddr.sun_path, serv_sock_path , sizeof(clientaddr.sun_path)-1);
	len = sizeof(clientaddr);

	pdbg("Client connect ---> %s <---", clientaddr.sun_path);
	ret = connect (clientfd, (struct sockaddr *)&clientaddr, len);
	if (ret < 0)
	{
		ploge("fail to connect");
		exit (EXIT_FAILURE);
	}

	return 0;
}

int
socket_server::start_client(void* param)
{
	char ch_recv[CONFIG_PROCMGR_MAXLEN];
	char ch_send[CONFIG_PROCMGR_MAXLEN];
	int bytes;
	int i0 = 0;
	proc_conf *mparam = (proc_conf*)param;
	// proc_conf mcli_send_param;
	// proc_conf mcli_recv_param;
	// process_manager mpm_send;
	// process_manager mpm_recv;
	proc_conf mproc_cli_param;
	process_manager mpm;
	serv_conf mserv;

	plogd("Enter: %s", __FUNCTION__);
	if (mparam == NULL) {
		ploge("param is NULL, abort!");
		return -EINVAL;
	}

	// for client param
		mproc_cli_param.listenfd = clientfd;
		mproc_cli_param.ppid     = clipid;
		mproc_cli_param.mlock    = mpm.get_thread_mutex();
		mproc_cli_param.flag     = mparam->flag; // | FLAG_BLOCK;
		mproc_cli_param.param    = cli_handler_param;
		mproc_cli_param.servcfg     = mparam->servcfg;
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
socket_server::stop_socket_server(void)
{
	return 0;
}

int
socket_server::stop_local_client(void)
{
	return 0;
}

int
socket_server::get_serv_fd(void)
{
	return servfd;
}

int
socket_server::get_listen_fd(void)
{
	return listenfd;
}

int
socket_server::get_client_fd(void)
{
	return clientfd;
}

int
socket_server::register_serv_msg_callback(void *(*proc_func)(void* param), void *param)
{
	msg_serv_callback = proc_func;
	msg_serv_conf = param;
	return 0;
}

int
socket_server::register_cli_recv_msg_callback(void *(*proc_func)(void* param), void* param)
{
	cli_recv_msg_callback = proc_func;
	msg_cli_param = param;
	return 0;
}

int
socket_server::register_cli_send_msg_callback(void *(*proc_func)(void* param), void* param)
{
	cli_send_msg_callback = proc_func;
	req_cli_param = param;
	return 0;
}


int
socket_server::register_serv_handler(void *(*proc_func)(void* param), void* param)
{
	serv_handler_callback = proc_func;
	serv_handler_param = param;
	return NULL;
}

int
socket_server::register_cli_handler(void *(*proc_func)(void* param), void* param)
{
	cli_handler_callback = proc_func;
	cli_handler_param = param;
	return NULL;
}

int
socket_server::set_serv_socket_path(char* spath)
{
	memset(serv_sock_path, 0x0, sizeof(serv_sock_path));
	sprintf(serv_sock_path, "%s", spath);
	return 0;
}

void
socket_server::get_serv_socket_path(char** spath)
{
	if (*spath == NULL) {
		return;
	}
	sprintf(*spath, "%s", serv_sock_path);
	return ;
}
