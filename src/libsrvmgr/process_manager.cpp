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
 *         Author:  Li Jin, lij1@xiaopeng.com
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
	process_param *mparam = (process_param*)param;
	int ret;

	log("fork, child pid: < %d >,\tcurrent pid: < %d >,\tparent pid: < %d >.\n", mp_id, getpid(), getppid());
	ret = pipe(pipefd);

	mp_id = fork();
	if (mp_id < 0)
	{
		loge("fork error");
		return mp_id;
	}
	else if (mp_id == 0)
	{
		logd("Sub process...");
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
		logd("Main process...");
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
	process_param *mparam = (process_param*)param;
	misc_utils mmisc;

	if (mparam != NULL) {
		pthread_mutex_lock(&mlock);
	}

	pthread_create(&mpth_id, NULL, start_routine, param);

	if ((mparam != NULL) && (mparam->flags & FLAG_SYNC_MUTEX) != FLAG_SYNC_MUTEX) {
		pthread_mutex_unlock(&mlock);
		pthread_mutex_lock(&mlock);
	}

	if (mparam != NULL) {
		pthread_mutex_unlock(&mlock);
	}

	if ((mparam->flags & FLAG_BLOCK) == FLAG_BLOCK) {
		logd("start_thread with block");
		pthread_join(mpth_id, &ret_param);
	}

	return 0;
}

int
process_manager::start_thread_sync_mutex(void* param)
{
	int ret;

	process_param *mparam = (process_param*)param;
	mparam->flags |= FLAG_SYNC_MUTEX;
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


