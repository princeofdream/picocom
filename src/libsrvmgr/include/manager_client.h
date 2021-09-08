/*
 * =====================================================================================
 *
 *       Filename:  manager_client.h
 *
 *    Description:  diagnosis client header
 *
 *        Version:  1.0
 *        Created:  03/18/2019 02:01:22 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, lij1@xiaopeng.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#ifndef __DIAGNOSIS_CLIENT_HEADER__
#define __DIAGNOSIS_CLIENT_HEADER__

#include <basic.h>
#include <messages_manager.h>


void* start_manager_client_proc(void*);
void* start_manager_client_recv_proc(void* param);

class manager_client
{
public:
	manager_client ();
	virtual ~manager_client ();

	int recv_message_async(message_param *param);
	int send_request_cmd(message_param *param);

	int send_request_cmd_once(message_param *param);

	// over write recv message
	int register_cli_recv_msg_callback(void *(*proc_func)(void* param), void *);
	// over write send message
	int register_cli_send_msg_callback(void *(*proc_func)(void* param), void *);

	void *(*cli_send_msg_callback)(void* param);
	void *req_cli_param;
	pthread_mutex_t sync_lock;

private:
	messages_manager mmsg;

};


#endif /* ifndef __DIAGNOSIS_CLIENT_HEADER__ */

