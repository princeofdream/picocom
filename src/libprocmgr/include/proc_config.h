/*
 * =====================================================================================
 *
 *       Filename:  proc_config.h
 *
 *    Description:  proc_config header
 *
 *        Version:  1.0
 *        Created:  2022年12月17日 00时23分27秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lijin (jin), jinli@syncore.space
 *   Organization:  SYNCORE
 *
 * =====================================================================================
 */

#ifndef __PROC_CONFIG_HEADER__
#define __PROC_CONFIG_HEADER__ 1

#include "procmgr_basic.h"
#include "serv_config.h"
#include "misc_utils.h"


typedef struct proc_conf_t {
	int servfd;
	int listenfd;
	int flag;
	pid_t ppid;
	pthread_mutex_t *mlock;
	char cmd[CONFIG_PROCMGR_MAXLEN];
	serv_conf *servcfg;
	void *param;
	void *sub_param;
	int pipefd[2];
} proc_conf;

class proc_config
{
public:
	proc_config ();
	virtual ~proc_config ();

	proc_conf *get_proc_conf();
	serv_conf *get_proc_serv();

	void set_proc_servfd(int);
	void set_proc_listenfd(int);
	void set_proc_ppid(pid_t);

	void set_proc_flag(uint32_t);
	void set_proc_mutex(pthread_mutex_t *);
	void set_proc_cmd(char*);

	void set_proc_serv(serv_conf *);
	void set_proc_param(void *);
	void set_proc_subparam(void *);

	int get_proc_servfd(int);
	int get_proc_listenfd(int);
	pid_t get_proc_ppid(pid_t);

	uint32_t get_proc_flag();
	pthread_mutex_t* get_proc_mutex();
	char* get_proc_cmd();

	void* get_proc_param();
	void* get_proc_subparam();

private:
	/* data */
	proc_conf pconf;
	pthread_mutex_t *mlock;
	int pipefd[2];
};

#endif /* ifndef __PROC_CONFIG_HEADER__ */


