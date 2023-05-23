/*
 * =====================================================================================
 *
 *       Filename:  socket_server.h
 *
 *    Description:  socket_server header
 *
 *        Version:  1.0
 *        Created:  03/07/2019 01:46:39 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#ifndef __LOCAL_SERVICE_HEADER__
#define __LOCAL_SERVICE_HEADER__

#include <procmgr_basic.h>
#include <process_manager.h>
#include <client_manager.h>
#include <misc_utils.h>

class socket_server_init
{
public:
	socket_server_init ();
	socket_server_init (char*);
	~socket_server_init ();

	void set_default_socket_server_path(char*);
private:
	/* data */
};

class socket_server
{
public:
	socket_server ();
	virtual ~socket_server ();

	static void* cli_handler(void* param);
	static void* serv_handler(void* param);

	int start_socket_server(void*);
	int stop_socket_server(void);
	int start_local_client(void*);
	int stop_local_client(void);

	int get_serv_fd(void);
	int get_listen_fd(void);
	int get_client_fd(void);

	int start_server(void*);
	int start_client(void*);

	int register_serv_msg_callback(void *(*proc_func)(void* param), void* param);
	int register_cli_recv_msg_callback(void *(*proc_func)(void* param), void* param);
	int register_cli_send_msg_callback(void *(*proc_func)(void* param), void* param);

	int register_serv_handler(void *(*proc_func)(void* param), void*);
	int register_cli_handler(void *(*proc_func)(void* param), void*);

private:
	static int process_message(char*, char**);
	int setup_server(void);
	int setup_client(void);
	int setup_ip_server(void*);
	int setup_ip_client(void*);

private:
	int servfd;
	int clientfd;
	int listenfd;

	int servpid;
	int clipid;

	socklen_t servlen, listenlen;
	struct sockaddr_un servaddr;
	struct sockaddr_un listenaddr;
	struct sockaddr_un clientaddr;

	struct sockaddr_in serv_ip_addr;
	struct sockaddr_in listen_ip_addr;
	struct sockaddr_in client_ip_addr;

	struct sigaction sa;
	pid_t lpid;
	pthread_t lpthid;

	void *(*msg_serv_callback)(void* param);
	void *(*cli_recv_msg_callback)(void* param);
	void *(*cli_send_msg_callback)(void* param);
	void *msg_serv_conf;
	void *msg_cli_param;
	void *req_cli_param;

	void *(*serv_handler_callback)(void* param);
	void *(*cli_handler_callback)(void* param);
	void *serv_handler_param;
	void *cli_handler_param;

	// void *(*serv_handler)(void* param);
	// void *(*cli_handler)(void* param);

public:
	int set_serv_socket_path(char* spath);
	void get_serv_socket_path(char** spath);

private:
	char serv_sock_path[CONFIG_PROCMGR_MAXLEN];

private:
	pthread_mutex_t serv_lock;
	pthread_mutex_t cli_lock;

};


#endif /* ifndef __LOCAL_SERVICE_HEADER__ */

