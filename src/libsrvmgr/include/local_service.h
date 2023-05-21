/*
 * =====================================================================================
 *
 *       Filename:  local_service.h
 *
 *    Description:  local_service header
 *
 *        Version:  1.0
 *        Created:  03/07/2019 01:46:39 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, lij1@xiaopeng.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#ifndef __LOCAL_SERVICE_HEADER__
#define __LOCAL_SERVICE_HEADER__

#include <procmgr_basic.h>
#include <process_manager.h>
#include <service_manager.h>
#include <manager_client.h>
#include <misc_utils.h>

class local_service
{
public:
	local_service ();
	virtual ~local_service ();

	int start_local_service(void*);
	int stop_local_service(void);
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
	int setup_server(void*);
	int setup_client(void*);
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
	void *msg_serv_param;
	void *msg_cli_param;
	void *req_cli_param;

	void *(*serv_handler_callback)(void* param);
	void *(*cli_handler_callback)(void* param);
	void *serv_handler_param;
	void *cli_handler_param;

	// void *(*serv_handler)(void* param);
	// void *(*cli_handler)(void* param);
};


#endif /* ifndef __LOCAL_SERVICE_HEADER__ */

