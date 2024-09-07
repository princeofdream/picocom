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

#include <cstdint>
#include <algorithm>
#include <iterator>
#include <array>
#include <mutex>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <ios>
#include <thread>

#ifndef WIN32
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/mman.h>

#ifndef __QNX__      //modify by jack.wang 2022.0704 for QNX system
#include <sys/shm.h>
#include <sys/prctl.h>
#endif
#endif // !WIN32


#define CONFIG_PROCMGR_MAXLEN 1024
#define CONFIG_PROCMGR_BUFFER_LEN 1024
#define CONFIG_PROCMGR_COMMON_LEN 64

#undef LOG_TAG
#define LOG_TAG "procmgr"

// #define uint32_t (unsigned int)
// #define uint16_t (unsigned short)
// #define uint8_t (unsigned char)

#ifndef SERVER_SOCKET_PATH
	#if defined(__i386__) || defined(__x86_64__)
		#define SERVER_SOCKET_PATH "/tmp"
	#elif defined(ANDROID) || defined(__aarch64__) || defined(__arm__)
		#define SERVER_SOCKET_PATH "/dev/socket"
	#else
		#define SERVER_SOCKET_PATH "."
	#endif
#endif

#ifndef SERVER_SOCKET_NAME
	#define SERVER_SOCKET_NAME "procmgr"
#endif

#define PROCMGR_MAX_VAL(var1, var2) ((var1 > var2)? var1:var2)

enum class LogLevel : uint8_t {
    _CLOSE = 0x00,
    L_FAT = 0x01,
    L_ERROR = 0x02,
    L_WARNING = 0x03,
    L_INFO = 0x04,
    L_DEBUG = 0x05
};

#ifndef CONFIG_DEFAULT_LOG_LEVEL
// #define CONFIG_DEFAULT_LOG_LEVEL LogLevel::L_DEBUG
#define CONFIG_DEFAULT_LOG_LEVEL LogLevel::L_INFO
// #define CONFIG_DEFAULT_LOG_LEVEL LogLevel::L_WARNING
// #define CONFIG_DEFAULT_LOG_LEVEL LogLevel::L_ERROR
// #define CONFIG_DEFAULT_LOG_LEVEL LogLevel::L_FAT
// #define CONFIG_DEFAULT_LOG_LEVEL LogLevel::_CLOSE
#endif

#if defined( __ANDROID__)
    #include "android/log.h"
    #define plogd(...) {if(pmgr_log::logOutput(DEBUG))  { __android_log_print(ANDROID_LOG_DEBUG, "SY_STREAMERD", __VA_ARGS__);}}
    #define plogi(...) {if(pmgr_log::logOutput(INFO))   { __android_log_print(ANDROID_LOG_INFO, "SY_STREAMERD", __VA_ARGS__);}}
    #define plogw(...) {if(pmgr_log::logOutput(WARNING)){ __android_log_print(ANDROID_LOG_WARN, "SY_STREAMERD", __VA_ARGS__);}}
    #define ploge(...) {if(pmgr_log::logOutput(ERROR))  { __android_log_print(ANDROID_LOG_ERROR, "SY_STREAMERD", __VA_ARGS__);}}
    #define plogc(...) {if(pmgr_log::logOutput(FAT))    { __android_log_print(ANDROID_LOG_ERROR, "SY_STREAMERD", __VA_ARGS__);}}  // ANDROID_LOG_FATAL

    #define pmgrLogD(...) {__android_log_print(ANDROID_LOG_DEBUG, "SY_STREAMERD", __VA_ARGS__);}
    #define pmgrLogI(...) {__android_log_print(ANDROID_LOG_INFO, "SY_STREAMERD", __VA_ARGS__);}
    #define pmgrLogW(...) {__android_log_print(ANDROID_LOG_WARN, "SY_STREAMERD", __VA_ARGS__);}
    #define pmgrLogE(...) {__android_log_print(ANDROID_LOG_ERROR, "SY_STREAMERD", __VA_ARGS__);}
    #define pmgrLogC(...) {__android_log_print(ANDROID_LOG_ERROR, "SY_STREAMERD", __VA_ARGS__);}  // ANDROID_LOG_FATAL
#elif defined(__QNX__)
    #include <sys/slog.h>
    #include <sys/slogcodes.h>
    #define _SLOGC_SYNCORE  (_SLOGC_3RDPARTY_OEM00001_START + 1)

    #define plogd(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_DEBUG))   {slogf(_SLOGC_SYNCORE, _SLOG_INFO,      "D [%s:%d] " frm, __FUNCTION__,__LINE__, ##args);}}
    #define plogi(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_INFO))    {slogf(_SLOGC_SYNCORE, _SLOG_INFO,       "I [%s:%d] " frm, __FUNCTION__,__LINE__, ##args);}}
    #define plogw(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_WARNING)) {slogf(_SLOGC_SYNCORE, _SLOG_WARNING, "W [%s:%d] " frm, __FUNCTION__,__LINE__, ##args);}}
    #define ploge(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_ERROR))   {slogf(_SLOGC_SYNCORE, _SLOG_ERROR,     "E [%s:%d] " frm, __FUNCTION__,__LINE__, ##args);}}
    #define plogc(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_FAT))     {slogf(_SLOGC_SYNCORE, _SLOG_CRITICAL,    "C [%s:%d] " frm, __FUNCTION__,__LINE__, ##args);}}

    #define pmgrLogD(frm, args...) {slogf(_SLOGC_SYNCORE, _SLOG_INFO,     "D " frm,  ##args);}
    #define pmgrLogI(frm, args...) {slogf(_SLOGC_SYNCORE, _SLOG_INFO,     "I " frm,  ##args);}
    #define pmgrLogW(frm, args...) {slogf(_SLOGC_SYNCORE, _SLOG_WARNING,  "W " frm, ##args);}
    #define pmgrLogE(frm, args...) {slogf(_SLOGC_SYNCORE, _SLOG_ERROR,    "E " frm, ##args);}
    #define pmgrLogC(frm, args...) {slogf(_SLOGC_SYNCORE, _SLOG_CRITICAL, "C " frm, ##args);}
#elif defined(__linux__)
    #ifndef CONFIG_LINUX_SYSLOG
    #define CONFIG_LINUX_SYSLOG 1
    #endif /* ifndef CONFIG_LINUX_SYSLOG */

    #if CONFIG_LINUX_SYSLOG
        #include <syslog.h>
        static  __attribute__((constructor)) void INIT_LOG()
        {
            openlog("", LOG_PID|LOG_PERROR|LOG_CONS, 0 );
        }
        #define plogd(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_DEBUG)) {syslog(LOG_USER|LOG_DEBUG,   "D [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}
        #define plogi(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_INFO)) {syslog(LOG_USER|LOG_INFO,    "I [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}
        #define plogw(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_WARNING)) {syslog(LOG_USER|LOG_WARNING, "W [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}
        #define ploge(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_ERROR)) {syslog(LOG_USER|LOG_ERR,     "E [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}
        #define plogc(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_FAT)) {syslog(LOG_USER|LOG_CRIT,    "E [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}

        #define pmgrLogD(frm, args...) {syslog(LOG_USER|LOG_DEBUG,   "D " frm " \n" , ##args);}
        #define pmgrLogI(frm, args...) {syslog(LOG_USER|LOG_INFO,    "I " frm " \n" , ##args);}
        #define pmgrLogW(frm, args...) {syslog(LOG_USER|LOG_WARNING, "W " frm " \n" , ##args);}
        #define pmgrLogE(frm, args...) {syslog(LOG_USER|LOG_ERR,     "E " frm " \n" , ##args);}
        #define pmgrLogC(frm, args...) {syslog(LOG_USER|LOG_CRIT,    "E " frm " \n" , ##args);}
    #else
        #define plogd(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_DEBUG)) {printf("D [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}
        #define plogi(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_INFO)) {printf("I [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}
        #define plogw(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_WARNING)) {printf("W [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}
        #define ploge(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_ERROR)) {printf("E [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}
        #define plogc(frm, args...) {if(pmgr_log::LogOutput(LogLevel::L_FAT)) {printf("C [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}

        #define pmgrLogD(frm, args...) {printf("D " frm " \n" , ##args);}
        #define pmgrLogI(frm, args...) {printf("I " frm " \n" , ##args);}
        #define pmgrLogW(frm, args...) {printf("W " frm " \n" , ##args);}
        #define pmgrLogE(frm, args...) {printf("E " frm " \n" , ##args);}
        #define pmgrLogC(frm, args...) {printf("C " frm " \n" , ##args);}
    #endif
#endif

#ifndef __QNX__     //modify by jack.wang 2022.0704 for QNX system
#define set_thread_name(name)   prctl(PR_SET_NAME, name, 0, 0, 0)
#define get_thread_name(name)   prctl(PR_GET_NAME, name)
#endif

#define CHECK_FOR_NEED_BUFF(buff_size_needed)   do {           \
        if (log_size >= (N - buff_size_needed)) {              \
            PrintOutExitingMessage() ;                         \
        }                                                      \
} while(0)

#define CHECK_FOR_DYNAMIC_NEED_BUFF(buff_size_needed)   do {   \
        if (log_size >= (N - buff_size_needed)) {              \
            PrintOutExitingMessage() ;                         \
        }                                                      \
} while(0)

#define MESSAGE_OVERFLOW_FAILURE() do { \
        if (log_size > N) {             \
            PrintOutExitingMessage();   \
            printf("%s, %d : unexpected things hapens, some message lost here", __FUNCTION__, __LINE__);  \
        } \
} while(0)

class pmgr_log
{
public:
    pmgr_log ()
    {
    }

    ~pmgr_log ()
    {
    }

	static int PMGRLog_Get_cur_system_time(char *CurTimeChr, int32_t buf_size)
    {
	    struct timeval tv;
	    struct tm n_time;

	    gettimeofday(&tv,NULL);
	    localtime_r(&tv.tv_sec,&n_time);

	    if (CurTimeChr == nullptr || buf_size < 32){
		    memset(CurTimeChr, 0, buf_size);
		    return -EINVAL;
	    }
        sprintf(CurTimeChr,"%02d:%02d:%02d.%d"
        	, n_time.tm_hour, n_time.tm_min,n_time.tm_sec,(int)tv.tv_usec);

        return 0;
    }

    static bool LogOutput(LogLevel lv)
    {
        if (lv <= CONFIG_DEFAULT_LOG_LEVEL) {
            return true;
        }
        return false;
    }

private:
    /* data */
};

#define LOG_BUFF_SIZE (512)

#define INFO    (LogLevel::L_INFO)
#define WARNING (LogLevel::L_WARNING)
#define ERROR   (LogLevel::L_ERROR)
#define DEBUG   (LogLevel::L_DEBUG)
#define FAT     (LogLevel::L_FAT)

#ifndef CONFIG_STREAMERD_GITVER
    #define CONFIG_STREAMERD_GITVER ""
#endif
#ifndef CONFIG_STREAMERD_GITSTAT
    #define CONFIG_STREAMERD_GITSTAT ""
#endif
#ifndef CONFIG_STREAMERD_BUILD_USER
    #define CONFIG_STREAMERD_BUILD_USER ""
#endif
#ifndef CONFIG_STREAMERD_BUILD_HOSTNAME
    #define CONFIG_STREAMERD_BUILD_HOSTNAME ""
#endif

#define version_info       "1.1.10.230626.182000.PMGR." \
    CONFIG_STREAMERD_GITVER "" CONFIG_STREAMERD_GITSTAT \
    "." __CONFIG_TIERONE__ \
    ".release("CONFIG_STREAMERD_BUILD_USER "@" CONFIG_STREAMERD_BUILD_HOSTNAME")"

#define showPMgrVersion()  plogw("%s",version_info);










#endif /* ifndef __PROCMGR_BASIC_HEADER__ */

