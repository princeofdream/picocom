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
 *         Author:  Li Jin, lij1@xiaopeng.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#include <messages_manager.h>

void*
process_callback(void* param)
{
	logd("[%s:%d] <%s>", __FILE__, __LINE__, __func__);
	return NULL;
}

messages_manager::messages_manager()
{
	serv_proc_callback    = NULL;
	cli_proc_callback     = NULL;
	mserv_param.sub_param = NULL;
	mcli_param.sub_param  = NULL;
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
		loge("[%s:%d] param is null, abort!", __func__,__LINE__);
		return NULL;
	}

	mserv_param.sockfd = mserv_msg->sockfd;
	mserv_param.value  = mserv_msg->value;
	mserv_param.msg    = mserv_msg->msg;
	memset(mserv_param.cmd, 0x0, sizeof(mserv_param.cmd));
	sprintf(mserv_param.cmd, "%s", mserv_msg->cmd);

	if (serv_proc_callback == NULL)
		serv_cmd_handler(&mserv_param);
	else
		serv_proc_callback(&mserv_param);

	sprintf(mserv_msg->cmd, "%s", mserv_param.cmd);
	return (void*)&mserv_param;
}

void*
messages_manager::serv_cmd_handler(message_param *param)
{
	message_param *msg_param = param;

	if (param == NULL) {
		loge("message param is NULL");
		return NULL;
	}
	// if (msg_param->sub_param == NULL)
	//     loge("custon param is NULL");

	if (strcmp(msg_param->cmd, "ack") == 0) {
		sprintf(msg_param->cmd, "%s %s", msg_param->cmd, "ok");
	} else if (strcmp(msg_param->cmd, "quit") == 0) {
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
		loge("message param is NULL");
		return NULL;
	}
	// if (msg_param->sub_param == NULL)
	//     loge("custon param is NULL");

	if (strcasecmp(msg_param->cmd, "quit QUIT") == 0) {
		memset(msg_param->cmd, 0x0, sizeof(msg_param->cmd));
		sprintf(msg_param->cmd, "%s", "quit");
	}

	return NULL;
}

void*
messages_manager::cli_proc_messages(int sockfd, char* msg, char** value)
{
	message_param *msg_ret = NULL;
	char get_msg[3][MAXLEN];

	if (msg == NULL)
		return NULL;

	memset(get_msg , 0x0 , sizeof(get_msg));

	sscanf(msg, "%s %s %s", get_msg[0], get_msg[1], get_msg[2]);

	DBG("get[0]: %s", get_msg[0]);
	DBG("get[1]: %s", get_msg[1]);
	DBG("get[2]: %s", get_msg[2]);

	mcli_param.sockfd = sockfd;
	mcli_param.value  = value;
	mcli_param.msg    = msg;
	// mcli_param.sub_param  = NULL;

	memset(mcli_param.cmd, 0x0, sizeof(mcli_param.cmd));

	if (cli_proc_callback == NULL) {
		cli_cmd_handler(&mcli_param);
	} else {
		cli_proc_callback(&mcli_param);
	}

	return (void*)&mcli_param;
}

int
messages_manager::register_serv_proc_callback(void *(*proc_func)(message_param* param), message_param *param)
{
	serv_proc_callback = proc_func;
	mserv_param.sub_param   = param;
	return 0;
}

int
messages_manager::register_cli_proc_callback(void *(*proc_func)(message_param* param), message_param *param)
{
	cli_proc_callback = proc_func;
	mcli_param.sub_param   = param;
	return 0;
}

void
messages_manager::dump_message_info(message_param *param)
{
	if (param == NULL)
		return;

	logd("sockfd:\t %d" , param->sockfd);
	logd("index:\t %d"  , param->index);
	logd("result:\t %d" , param->result);
	logd("count:\t %d"  , param->count);
	logd("cmd:\t %s"    , param->cmd);
	logd("value:\t %s"  , *param->value);
	misc_utils mmisc;
	mmisc.flag_to_string(param->flags, "msg");

	logd("cmd: %s"    , param->cmd);
	return;
}

message_param *
messages_manager::get_msg_serv_param(void)
{
	return &mserv_param;
}

message_param *
messages_manager::get_msg_cli_param(void)
{
	return &mcli_param;
}

void
messages_manager::set_msg_serv_param(message_param *param)
{
	memcpy(&mcli_param, param, sizeof(message_param));
	return ;
}

void
messages_manager::set_msg_cli_param(message_param *param)
{
	memcpy(&mserv_param, param, sizeof(message_param));
	return ;
}




