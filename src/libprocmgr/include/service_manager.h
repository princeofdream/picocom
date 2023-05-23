/*
 * =====================================================================================
 *
 *       Filename:  service_manager.h
 *
 *    Description:  service_manager header
 *
 *        Version:  1.0
 *        Created:  03/18/2019 11:12:24 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */

#ifndef __DIAGNOSOS_SERVER_HEADER__
#define __DIAGNOSOS_SERVER_HEADER__


#include <procmgr_basic.h>
#include <serv_config.h>

#include <messages_manager.h>
#include <socket_server.h>

/*! \enum service_manager_type_t
 *
 *  Detailed description
 */
enum service_manager_type_t {
	MGR_SERVICE_SERV = 0,
	MGR_SERVICE_CLI
};

class service_manager
{
public:
	service_manager ();
	virtual ~service_manager ();

	// start service manager
	static void* start_service_manager_proc(void*);
	static void* manager_cli_handler(void* param);

	int start_manager_server(void*);
	int start_client_manager(void*);

	static int msg_handler(struct sockaddr_nl *nl, struct nlmsghdr *msg);
	int respond_cmd_async(int, void*);
	int respond_cmd(int, void*);

	int recv_message_async(message_param *);
	int send_request_cmd_once(message_param *);
	int send_request_cmd(message_param *);

	void set_service_type(int);

	int start_client_manager_recv_pthread(message_param*);
	int start_client_manager_send_pthread(message_param*);

public:
	// over write serv message
	int register_serv_msg_callback(void *(*proc_func)(void* param), void*);
	// over write recv message
	int register_cli_recv_msg_callback(void *(*proc_func)(void* param), void *);
	// over write send message
	int register_cli_send_msg_callback(void *(*proc_func)(void* param), void *);

	void *(*cli_send_msg_callback)(void* param);
	void *req_cli_param;

	pthread_mutex_t sync_lock;
	int cli_recv_ready;

	int service_type;
	socket_server ml_serv;

private:
	messages_manager mmsg;

};


#endif /* ifndef __DIAGNOSOS_SERVER_HEADER__ */

