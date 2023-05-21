/*
 * =====================================================================================
 *
 *       Filename:  process_manager.h
 *
 *    Description:  process_manager header
 *
 *        Version:  1.0
 *        Created:  03/07/2019 09:53:06 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, lij1@xiaopeng.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#ifndef __PROCESS_MANAGER_HEADER__
#define __PROCESS_MANAGER_HEADER__

#include <procmgr_basic.h>
#include <misc_utils.h>

class process_manager
{
public:
	process_manager ();
	process_manager (void *(*sub_proc)(void *));
	virtual ~process_manager ();

	// int start_process(void* , void *(*start_routine)(void *));
	int start_process(void*);
	int stop_process(pid_t);
	// int start_thread(void* , void *(*start_routine)(void *));
	int start_thread(void*);
	int start_thread_sync_mutex(void*);
	int stop_thread(pthread_t);

	pid_t get_process_id();
	pid_t get_process_parent_id();
	pthread_t get_thread_id();
	pthread_t get_thread_parent_id();
	pthread_mutex_t* get_thread_mutex();
	int get_pipe_fd_in();
	int get_pipe_fd_out();

	void *(*start_routine)(void *);

private:
	pid_t mp_id;
	pid_t mpp_id;

	pthread_t mpth_id;
	pthread_t mppth_id;

	pthread_mutex_t mlock;
	int pipefd[2];
};


#endif /* ifndef __PROCESS_MANAGER_HEADER__ */

