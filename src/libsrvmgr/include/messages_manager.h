/*
 * =====================================================================================
 *
 *       Filename:  messages_manager.h
 *
 *    Description:  messages_manager header
 *
 *        Version:  1.0
 *        Created:  03/18/2019 02:13:27 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, lij1@xiaopeng.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#ifndef __MESSAGE_MANAGER_HEADER__
#define __MESSAGE_MANAGER_HEADER__

#include <basic.h>

class messages_manager
{
public:
	messages_manager ();
	virtual ~messages_manager ();


	void* serv_proc_messages(message_param*);
	void* cli_proc_messages(int, char*, char**);
	int register_serv_proc_callback(void *(*proc_func)(void* param), void*);
	int register_cli_proc_callback(void *(*proc_func)(void* param), void*);
	void dump_message_info(message_param *param);
	void* serv_cmd_handler(message_param *param);
	void* cli_cmd_handler(message_param *param);

private:
	message_param mcli_param;
	message_param mserv_param;
	void *(*serv_proc_callback)(void* param);
	void *(*cli_proc_callback)(void* param);

};


#endif /* ifndef __MESSAGE_MANAGER_HEADER__ */



