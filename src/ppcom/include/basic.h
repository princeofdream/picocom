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


#include <arpa/inet.h>

#ifdef ANDROID
#include <cutils/log.h>
#include <cutils/properties.h>
#endif

#undef LOG_TAG
#define LOG_TAG "srvmgr"

#define DEBUG

#ifndef MAX_VAL
	#define MAX_VAL(var1, var2) ((var1 > var2)? var1:var2)
#endif

#endif /* ifndef __BASIC_HEADER__ */
