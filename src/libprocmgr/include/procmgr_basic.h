/*
 * =====================================================================================
 *
 *       Filename:  procmgr_basic.h
 *
 *    Description:  basic env header
 *
 *        Version:  1.0
 *        Created:  03/05/2019 02:18:08 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */

#ifndef __PROCMGR_BASIC_HEADER__
#define __PROCMGR_BASIC_HEADER__

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


#include <arpa/inet.h>

#ifdef ANDROID
#include <cutils/log.h>
// #include <cutils/properties.h>
#endif

#define CONFIG_PROCMGR_MAXLEN 1024
#define CONFIG_PROCMGR_BUFFER_LEN 1024
#define CONFIG_PROCMGR_COMMON_LEN 64

#undef LOG_TAG
#define LOG_TAG "procmgr"

// #define uint32_t (unsigned int)
// #define uint16_t (unsigned short)
// #define uint8_t (unsigned char)

#define DEBUG


#undef SERVER_SOCKET_PATH
#if defined(__i386__) || defined(__x86_64__)
	#define SERVER_SOCKET_PATH "/tmp/procmgr.sock"
#elif defined(ANDROID) || defined(__aarch64__) || defined(__arm__)
	#define SERVER_SOCKET_PATH "/dev/socket/procmgr.sock"
#else
	#define SERVER_SOCKET_PATH "procmgr.sock"
#endif

// this is used to output log to STDIN
#define plog(frm,args...) printf(frm,##args)

#ifdef DEBUG
	#ifdef ANDROID
		#define pdbg(frm,args...) ALOGD("<%s:%d>---" frm "\n",__FUNCTION__,__LINE__,##args)
		#define ploge(frm,args...) ALOGE(frm,##args)
		#define plogd(frm,args...) ALOGD(frm,##args)
	#else
		#define pdbg(frm,args...) printf("<%s:%d>---" frm "\n",__FUNCTION__,__LINE__,##args)
		#define ploge(frm,args...) printf(frm"\n",##args)
		// #define plogd(frm,args...) printf(frm"\n",##args)
		#define plog(frm,args...) printf(frm"\n",##args)
		#define plogd(frm,args...) printf("<%s:%d>---" frm "\n",__FUNCTION__,__LINE__,##args)
	#endif

#else
	#ifdef ANDROID
		// #define pdbg(frm, args...) while(0){;}
		#define pdbg(frm,args...) printf(frm"\n",##args)
		#define ploge(frm,args...) ALOGE(frm,##args)
		#define plogd(frm,args...) ALOGD(frm,##args)
	#else
		// #define pdbg(frm, args...) while(0){;}
		#define pdbg(frm,args...) printf(frm"\n",##args)
		#define ploge(frm,args...) printf(frm"\n",##args)
		#define plogd(frm,args...) printf(frm"\n",##args)
	#endif
#endif


#define PROCMGR_MAX_VAL(var1, var2) ((var1 > var2)? var1:var2)

#endif /* ifndef __PROCMGR_BASIC_HEADER__ */

