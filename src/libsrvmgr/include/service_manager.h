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
 *         Author:  Li Jin, lij1@xiaopeng.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */

#ifndef __DIAGNOSOS_SERVER_HEADER__
#define __DIAGNOSOS_SERVER_HEADER__


#include <basic.h>
#include <messages_manager.h>
#include <local_service.h>

/*! \enum service_manager_type_t
 *
 *  Detailed description
 */
enum service_manager_type_t {
	MGR_SERVICE_SERV = 0,
	MGR_SERVICE_CLI
};

void* start_service_manager_proc(void*);

class service_manager
{
public:
	service_manager ();
	virtual ~service_manager ();

	int start_manager_server(void*);
	int start_manager_client(void*);

	static int msg_handler(struct sockaddr_nl *nl, struct nlmsghdr *msg);
	int respond_cmd_async(int, void*);
	int respond_cmd(int, void*);

	int recv_message_async(message_param *);
	int send_request_cmd_once(message_param *);
	int send_request_cmd(message_param *);

	void set_service_type(int);

	int start_manager_client_recv_pthread(message_param*);
	int start_manager_client_send_pthread(message_param*);
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

private:
	messages_manager mmsg;

};


#endif /* ifndef __DIAGNOSOS_SERVER_HEADER__ */

