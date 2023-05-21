/*
 * =====================================================================================
 *
 *       Filename:  proc_config.cpp
 *
 *    Description:  proc_config
 *
 *        Version:  1.0
 *        Created:  2022年12月17日 00时29分42秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lijin (jin), jinli@syncore.space
 *   Organization:  SYNCORE
 *
 * =====================================================================================
 */

#include <proc_config.h>


proc_config::proc_config()
{
	pconf.param = NULL;
	pconf.sub_param = NULL;
	pipefd[0] = -1;
	pipefd[1] = -1;
	mlock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mlock, NULL);
}

proc_config::~proc_config()
{
	free(mlock);
}

proc_conf*
proc_config::get_proc_conf()
{
	return &pconf;
}

serv_conf*
proc_config::get_proc_serv()
{
	return pconf.serv;
}

void
proc_config::set_proc_servfd (int val)
{
	pconf.servfd = val;
}

void
proc_config::set_proc_listenfd (int val)
{
	pconf.listenfd = val;
}

void
proc_config::set_proc_ppid(pid_t val)
{
	pconf.ppid = val;
}

void
proc_config::set_proc_flag(uint32_t val)
{
	pconf.flag = val;
}


void
proc_config::set_proc_mutex(pthread_mutex_t* lock)
{
	pconf.mlock = lock;
}

void
proc_config::set_proc_cmd(char* cmd)
{
	memset(pconf.cmd, 0x0, sizeof(pconf.cmd));
	sprintf(pconf.cmd, "%s", cmd);
}

void
proc_config::set_proc_param(void *param)
{
	pconf.param = param;
}

void
proc_config::set_proc_subparam(void *param)
{
	pconf.sub_param = param;
}

void
proc_config::set_proc_serv(serv_conf *param)
{
	pconf.serv = param;
}

int
proc_config::get_proc_servfd(int)
{
	return pconf.servfd;
}

int
proc_config::get_proc_listenfd(int)
{
	return pconf.listenfd;
}

pid_t
proc_config::get_proc_ppid(pid_t)
{
	return pconf.ppid;
}

uint32_t
proc_config:: get_proc_flag()
{
	return pconf.flag;
}

pthread_mutex_t*
proc_config::get_proc_mutex()
{
	return pconf.mlock;
}

char*
proc_config::get_proc_cmd()
{
	return pconf.cmd;
}

void*
proc_config::get_proc_param()
{
	return pconf.param;
}

void*
proc_config::get_proc_subparam()
{
	return pconf.sub_param;
}


