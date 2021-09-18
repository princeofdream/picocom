/*
 * =====================================================================================
 *
 *       Filename:  basic.h
 *
 *    Description:  basic env header
 *
 *        Version:  1.0
 *        Created:  03/05/2019 02:18:08 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, lij1@xiaopeng.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */

#ifndef __BASIC_HEADER__
#define __BASIC_HEADER__

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <sys/select.h>

#include <asm/types.h>
#include <linux/netlink.h>

#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <dirent.h>
#include <errno.h>
#include <malloc.h>
#include <net/if.h>
#include <sys/socket.h>
#include <functional>
#include <map>
#include <vector>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <misc_utils.h>


#include <arpa/inet.h>

#ifdef ANDROID
#include <cutils/log.h>
#include <cutils/properties.h>
#endif

#define MAXLEN 1024
#define READ_BUFFER_LEN 1024
#define COMMON_LEN 64

#undef LOG_TAG
#define LOG_TAG "vehicle_diagnosis"

// #define DEBUG


#if defined(__i386__) || defined(__x86_64__)
	#define SOCKET_SERVER_NAME "/tmp/srvmgr.sock"
#elif defined(ANDROID) || defined(__aarch64__) || defined(__arm__)
	#define SOCKET_SERVER_NAME "/dev/socket/srvmgr.sock"
#else
	#define SOCKET_SERVER_NAME "srvmgr.sock"
#endif

// this is used to output log to STDIN
#define log(frm,args...) printf(frm,##args)

// #ifdef DEBUG
// #ifdef ANDROID
// #define DBG(frm,args...) ALOGD("--<%s:%d>---" frm "---\n",__FUNCTION__,__LINE__,##args)
// #define loge(frm,args...) ALOGE(frm,##args)
// #define logd(frm,args...) ALOGD("--<%s:%d>---" frm "---\n",__FUNCTION__,__LINE__,##args)
// #else
#define DBG(frm,args...) printf("[ %s:%d ]\t---" frm "\n",__FILE__,__LINE__,##args)
#define loge(frm,args...) printf("[ %s:%d ]\t---" frm "\n",__FILE__,__LINE__,##args)
#define logd(frm,args...) printf("[ %s:%d ]\t---" frm "\n",__FILE__,__LINE__,##args)
// #endif
// #else
// #ifdef ANDROID
// #define DBG(frm, args...) while(0){;}
// #define loge(frm,args...) ALOGE(frm,##args)
// #define logd(frm,args...) ALOGD(frm,##args)
// #else
// #define DBG(frm, args...) while(0){;}
// #define loge(frm,args...) printf(frm"\n",##args)
// #define logd(frm,args...) printf(frm"\n",##args)
// #endif
// #endif


#define MAX_VAL(var1, var2) ((var1 > var2)? var1:var2)

typedef struct serv_param_t {
	char		socket_path[MAXLEN];
	int			port;
	uint32_t	flags;
	void*		serv_cls;
	void*		serv_prm;
	void*		serv_ext;
} serv_param;

typedef struct process_param_t {
	int				servfd;
	int				listenfd;
	uint32_t		flags;
	pid_t			ppid;
	pthread_mutex_t	*mlock;
	char			cmd[1024];
	serv_param		*serv;
	void			*param;
	int				pipefd[2];
} process_param;

typedef struct message_param_t {
	int			sockfd;
	int			index;
	int			count;
	uint32_t	flags;
	char		cmd[1024];
	char		**value;
	void		*param;
	char		*msg;
	int			result;
} message_param;

/*! \enum process_flags_t
 */
enum process_flags_t {
	FLAG_DEFAULT      = 0,
	FLAG_SYNC_MUTEX   = 0x1 << 0,
	FLAG_BLOCK        = 0x1 << 1,
	FLAG_WITH_PROCESS = 0x1 << 2,
	FLAG_WITH_PTHREAD = 0x1 << 3,
	FLAG_WITH_IP      = 0x1 << 4,
	FLAG_WITH_REPLY   = 0x1 << 5,
	FLAG_MSG          = 0x1 << 6,
	FLAG_MSG_ASYNC    = 0x1 << 7
};

#endif /* ifndef __BASIC_HEADER__ */
