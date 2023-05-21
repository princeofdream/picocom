/*
 * =====================================================================================
 *
 *       Filename:  process_manager.cpp
 *
 *    Description:  process manager
 *
 *        Version:  1.0
 *        Created:  03/07/2019 09:52:31 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#include <process_manager.h>

process_manager::process_manager ()
{
	pthread_mutex_init(&mlock, NULL);
}

process_manager::process_manager (void *(*sub_proc)(void *))
{
	start_routine = sub_proc;
	pthread_mutex_init(&mlock, NULL);
}

process_manager::~process_manager ()
{
}

int
process_manager::start_process(void* param)
{
	proc_conf *mparam = (proc_conf*)param;
	int ret;

	plog("fork, child pid: < %d >,\tcurrent pid: < %d >,\tparent pid: < %d >.\n", mp_id, getpid(), getppid());
	ret = pipe(pipefd);

	mp_id = fork();
	if (mp_id < 0)
	{
		ploge("fork error");
		return mp_id;
	}
	else if (mp_id == 0)
	{
		plogd("Sub process...");
		if (mparam != NULL) {
			mparam->pipefd[0] = pipefd[0];
			mparam->pipefd[1] = pipefd[1];
		}
		start_routine(mparam);
		exit(0);
	}
#if 1
	else
	{
		plogd("Main process...");
	}
#endif
	return (int)mp_id;
}

int
process_manager::stop_process(pid_t mpid)
{
	return 0;
}

int
process_manager::start_thread(void* param)
{
	void* ret_param;
	proc_conf *mparam = (proc_conf*)param;
	misc_utils mmisc;

	if (mparam != NULL) {
		pthread_mutex_lock(&mlock);
	}

	pthread_create(&mpth_id, NULL, start_routine, param);

	if ((mparam != NULL) && (mparam->flag & FLAG_SYNC_MUTEX) != FLAG_SYNC_MUTEX) {
		pthread_mutex_unlock(&mlock);
	}

	pthread_mutex_lock(&mlock);
	pthread_mutex_unlock(&mlock);

	if ((mparam->flag & FLAG_BLOCK) == FLAG_BLOCK) {
		pthread_join(mpth_id, &ret_param);
	}

	return 0;
}

int
process_manager::start_thread_sync_mutex(void* param)
{
	int ret;

	proc_conf *mparam = (proc_conf*)param;
	mparam->flag |= FLAG_SYNC_MUTEX;
	ret = start_thread(param);
	return ret;
}

int
process_manager::stop_thread(pthread_t mpthid)
{
	return 0;
}


pid_t
process_manager::get_process_id()
{
	return mp_id;
}

pid_t
process_manager::get_process_parent_id()
{
	return mpp_id;
}

pthread_t
process_manager::get_thread_id()
{
	return mpth_id;
}

pthread_t
process_manager::get_thread_parent_id()
{
	return mppth_id;
}

pthread_mutex_t*
process_manager::get_thread_mutex()
{
	return &mlock;
}

int
process_manager::get_pipe_fd_in()
{
	return pipefd[0];
}

int
process_manager::get_pipe_fd_out()
{
	return pipefd[1];
}

void
process_manager::init_serv_config()
{
	sconfig.set_serv_ip(CONFIG_DEFAULT_SERV_IP);
	sconfig.set_serv_port(CONFIG_DEFAULT_SERV_PORT);
	sconfig.set_serv_flag(0);
	sconfig.set_serv_source(NULL);
	sconfig.set_serv_primary_param(NULL);
	sconfig.set_serv_extern_param(NULL);
}

serv_config *
process_manager::get_serv_config()
{
	return &sconfig;
}

void
process_manager::init_proc_config()
{
	pconfig.set_proc_servfd(-1);
	pconfig.set_proc_listenfd(-1);
	pconfig.set_proc_ppid(-1);

	pconfig.set_proc_flag(FLAG_SYNC_MUTEX|
						  FLAG_BLOCK|
						  FLAG_WITH_PTHREAD);

	pconfig.set_proc_mutex(&mlock);
	pconfig.set_proc_cmd("");

	pconfig.set_proc_serv(sconfig.get_serv_conf());
}

proc_config *
process_manager::get_proc_config()
{
	return &pconfig;
}

int
process_manager::register_routine_callback(void *(*proc_func)(void* param), void* param)
{
	start_routine = proc_func;
	return 0;
}


