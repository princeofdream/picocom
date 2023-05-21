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

#include <procmgr_basic.h>

class messages_manager
{
public:
	messages_manager ();
	virtual ~messages_manager ();


	void* serv_proc_messages(message_param*);
	void* cli_proc_messages(int, char*, char**);
	int register_serv_proc_callback(void *(*proc_func)(message_param* param), message_param*);
	int register_cli_proc_callback(void *(*proc_func)(message_param* param), message_param*);
	void dump_message_info(message_param *param);
	void* serv_cmd_handler(message_param *param);
	void* cli_cmd_handler(message_param *param);

	message_param *get_msg_serv_param(void);
	message_param *get_msg_cli_param(void);
	void set_msg_serv_param(message_param *);
	void set_msg_cli_param(message_param *);

private:
	message_param mcli_param;
	message_param mserv_param;
	void *(*serv_proc_callback)(message_param* param);
	void *(*cli_proc_callback)(message_param* param);

};


#endif /* ifndef __MESSAGE_MANAGER_HEADER__ */



