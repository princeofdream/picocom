/*
 * =====================================================================================
 *
 *       Filename:  messages_manager.cpp
 *
 *    Description:  messages_manager
 *
 *        Version:  1.0
 *        Created:  03/18/2019 02:12:57 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#include <messages_manager.h>

void*
process_callback(void* param)
{
	plogd("[%s:%d] <%s>", __FILE__, __LINE__, __func__);
	return NULL;
}

messages_manager::messages_manager()
{
	serv_proc_callback = NULL;
	cli_proc_callback  = NULL;
	mserv_conf.param   = NULL;
	mcli_param.param    = NULL;
	// serv_proc_callback = process_callback;
	// cli_proc_callback  = process_callback;
}

messages_manager::~messages_manager()
{
}

void*
messages_manager::serv_proc_messages(message_param *param)
{
	message_param *mserv_msg = param;

	if (param == NULL) {
		ploge("[%s:%d] param is null, abort!", __func__,__LINE__);
		return NULL;
	}

	// plogd("%s:%d, %#x", __FILE__,__LINE__, &mserv_conf);
	mserv_conf.sockfd = mserv_msg->sockfd;
	mserv_conf.value  = mserv_msg->value;
	mserv_conf.msg    = mserv_msg->msg;
	memset(mserv_conf.cmd, 0x0, sizeof(mserv_conf.cmd));
	sprintf(mserv_conf.cmd, "%s", mserv_msg->cmd);

	if (serv_proc_callback == NULL)
		serv_cmd_handler(&mserv_conf);
	else
		serv_proc_callback(&mserv_conf);

	sprintf(mserv_msg->cmd, "%s", mserv_conf.cmd);
	return (void*)&mserv_conf;
}

void*
messages_manager::serv_cmd_handler(message_param *param)
{
	message_param *msg_param = param;

	if (param == NULL) {
		ploge("message param is NULL");
		return NULL;
	}
	// if (msg_param->param == NULL)
	//     ploge("custon param is NULL");

	memset(msg_param->cmd, 0x0, sizeof(msg_param->cmd));
	if (strcmp(msg_param->msg, "ack") == 0) {
		sprintf(msg_param->cmd, "%s %s", msg_param->cmd, "ok");
	} else if (strcmp(msg_param->msg, "quit") == 0) {
		sprintf(msg_param->cmd, "%s %s", msg_param->cmd, "QUIT");
	} else {
		sprintf(msg_param->cmd, "%s %s", msg_param->cmd, "NG");
	}

	return NULL;
}

void*
messages_manager::cli_cmd_handler(message_param *param)
{
	message_param *msg_param = param;

	if (param == NULL) {
		ploge("message param is NULL");
		return NULL;
	}
	// if (msg_param->param == NULL)
	//     ploge("custon param is NULL");

	memset(msg_param->cmd, 0x0, sizeof(msg_param->cmd));
	if (strcasecmp(msg_param->msg, "quit QUIT") == 0) {
		sprintf(msg_param->cmd, "%s", "quit");
	}

	return NULL;
}

void*
messages_manager::cli_proc_messages(int sockfd, char* msg, char** value)
{
	message_param *msg_ret = NULL;
	char get_msg[3][CONFIG_PROCMGR_MAXLEN];

	if (msg == NULL)
		return NULL;

	memset(get_msg , 0x0 , sizeof(get_msg));

	sscanf(msg, "%s %s %s", get_msg[0], get_msg[1], get_msg[2]);

	pdbg("get[0]: %s", get_msg[0]);
	pdbg("get[1]: %s", get_msg[1]);
	pdbg("get[2]: %s", get_msg[2]);

	mcli_param.sockfd = sockfd;
	mcli_param.value  = value;
	mcli_param.msg    = msg;
	// mcli_param.param  = NULL;

	memset(mcli_param.cmd, 0x0, sizeof(mcli_param.cmd));

	if (cli_proc_callback == NULL) {
		cli_cmd_handler(&mcli_param);
	} else {
		cli_proc_callback(&mcli_param);
	}

	return (void*)&mcli_param;
}

int
messages_manager::register_serv_proc_callback(void *(*proc_func)(void* param), void *param)
{
	serv_proc_callback = proc_func;
	mserv_conf.param   = param;
	return 0;
}

int
messages_manager::register_cli_proc_callback(void *(*proc_func)(void* param), void *param)
{
	cli_proc_callback = proc_func;
	mcli_param.param   = param;
	return 0;
}

void
messages_manager::dump_message_info(message_param *param)
{
	if (param == NULL)
		return;

	plogd("sockfd:\t %d" , param->sockfd);
	plogd("index:\t %d"  , param->index);
	plogd("result:\t %d" , param->result);
	plogd("count:\t %d"  , param->count);
	plogd("cmd:\t %s"    , param->cmd);
	plogd("value:\t %s"  , *param->value);
	misc_utils mmisc;
	mmisc.flag_to_string(param->flags, "msg");

	plogd("cmd: %s"    , param->cmd);
	return;
}

