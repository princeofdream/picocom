#pragma once

#ifndef __LIBFFWD_DBGLOG_H__
#define __LIBFFWD_DBGLOG_H__

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
#include <fcntl.h>
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

#ifndef TASK_ID_MAX
#define TASK_ID_MAX 0x7fff
#endif /* ifndef TASK_ID_MAX */

// #include "Infrastructure_Tool/span.hpp"
// #include "APF/ScopeGuard.h"
// #include "Infrastructure_Tool/TinyXMLparse.h"
#include <string.h>

#ifdef _CATCH_TEST_MODE_
#include "catch.hpp"
#endif

enum LogLevel {
    LOGLEVEL_L_CLOSE   = 0x00,
    LOGLEVEL_L_FAT     = 0x01,
    LOGLEVEL_L_ERROR   = 0x02,
    LOGLEVEL_L_WARNING = 0x03,
    LOGLEVEL_L_INFO    = 0x04,
    LOGLEVEL_L_DEBUG   = 0x05
};

#ifndef CONFIG_LOG_STD_OUTPUT
#define CONFIG_LOG_STD_OUTPUT 0
#endif
// #include "osa/com_osa_mutex.h"

#if defined( __ANDROID__)
    #include "android/log.h"
    #define qLogD(...) {if(dbglog::logOutput(DEBUG)){ __android_log_print(ANDROID_LOG_DEBUG, "LIBFFWD", __VA_ARGS__);}}
    #define qLogI(...) {if(dbglog::logOutput(INFO)){ __android_log_print(ANDROID_LOG_INFO, "LIBFFWD", __VA_ARGS__);}}
    #define qLogW(...) {if(dbglog::logOutput(WARNING)){ __android_log_print(ANDROID_LOG_WARN, "LIBFFWD", __VA_ARGS__);}}
    #define qLogE(...) {if(dbglog::logOutput(ERROR)){ __android_log_print(ANDROID_LOG_ERROR, "LIBFFWD", __VA_ARGS__);}}
    #define qLogC(...) {if(dbglog::logOutput(FAT)){ __android_log_print(ANDROID_LOG_ERROR, "LIBFFWD", __VA_ARGS__);}}  // ANDROID_LOG_FATAL

    #define pLogD(...) {__android_log_print(ANDROID_LOG_DEBUG, "LIBFFWD", __VA_ARGS__);}
    #define pLogI(...) {__android_log_print(ANDROID_LOG_INFO, "LIBFFWD", __VA_ARGS__);}
    #define pLogW(...) {__android_log_print(ANDROID_LOG_WARN, "LIBFFWD", __VA_ARGS__);}
    #define pLogE(...) {__android_log_print(ANDROID_LOG_ERROR, "LIBFFWD", __VA_ARGS__);}
    #define pLogC(...) {__android_log_print(ANDROID_LOG_ERROR, "LIBFFWD", __VA_ARGS__);}  // ANDROID_LOG_FATAL
#elif defined(__QNX__)
    #include <sys/slog.h>
    #include <sys/slogcodes.h>
    #define _SLOGC_SYNCORE  (_SLOGC_3RDPARTY_OEM00001_START + 1)

    #define LOGCOMMON(frm, loglev, args...) {if(dbglog::LogOutput(loglev)) { \
        int LOGLEV = _SLOG_INFO; char LOGTAG = 'I'; \
        if(LOGLEVEL_L_DEBUG == loglev) {LOGLEV = _SLOG_DEBUG1; LOGTAG = 'D';} \
        else if(LOGLEVEL_L_INFO == loglev) {LOGLEV = _SLOG_INFO; LOGTAG = 'I';} \
        else if(LOGLEVEL_L_WARNING == loglev) {LOGLEV = _SLOG_WARNING; LOGTAG = 'W';} \
        else if(LOGLEVEL_L_ERROR == loglev) {LOGLEV = _SLOG_ERROR; LOGTAG = 'E';} \
        else if(LOGLEVEL_L_FAT == loglev) {LOGLEV = _SLOG_CRITICAL; LOGTAG = 'C';} \
        slogf(_SLOGC_SYNCORE, LOGLEV,      "%c [%s:%d] [T%02d]" frm, LOGTAG, __FUNCTION__,__LINE__, std::this_thread::get_id(), ##args); \
        if(CONFIG_LOG_STD_OUTPUT) { printf("%c [%s:%d] [T%02d]" frm " \n" , LOGTAG, __FUNCTION__,__LINE__,std::this_thread::get_id(), ##args);} \
    }}
    #define LOGCOMMON_WITHSTD(frm, loglev, args...) {if(dbglog::LogOutput(loglev)) { \
        int LOGLEV = _SLOG_INFO; char LOGTAG = 'I'; \
        if(LOGLEVEL_L_DEBUG == loglev) {LOGLEV = _SLOG_DEBUG1; LOGTAG = 'D';} \
        else if(LOGLEVEL_L_INFO == loglev) {LOGLEV = _SLOG_INFO; LOGTAG = 'I';} \
        else if(LOGLEVEL_L_WARNING == loglev) {LOGLEV = _SLOG_WARNING; LOGTAG = 'W';} \
        else if(LOGLEVEL_L_ERROR == loglev) {LOGLEV = _SLOG_ERROR; LOGTAG = 'E';} \
        else if(LOGLEVEL_L_FAT == loglev) {LOGLEV = _SLOG_CRITICAL; LOGTAG = 'C';} \
        slogf(_SLOGC_SYNCORE, LOGLEV,      "%c [%s:%d] [T%02d]" frm, LOGTAG, __FUNCTION__,__LINE__, std::this_thread::get_id(), ##args); \
        if(1) { printf("%c [%s:%d] [T%02d]" frm " \n" , LOGTAG, __FUNCTION__,__LINE__,std::this_thread::get_id(), ##args);} \
    }}


    #define qLogD(frm, args...) LOGCOMMON(frm, LOGLEVEL_L_DEBUG, ##args);
    #define qLogI(frm, args...) LOGCOMMON(frm, LOGLEVEL_L_INFO, ##args);
    #define qLogW(frm, args...) LOGCOMMON(frm, LOGLEVEL_L_WARNING, ##args);
    #define qLogE(frm, args...) LOGCOMMON(frm, LOGLEVEL_L_ERROR, ##args);
    #define qLogC(frm, args...) LOGCOMMON(frm, LOGLEVEL_L_FAT, ##args);
    #define RLOGW(frm, args...) LOGCOMMON_WITHSTD(frm, LOGLEVEL_L_WARNING, ##args);
    #define RLOGE(frm, args...) LOGCOMMON_WITHSTD(frm, LOGLEVEL_L_ERROR, ##args);

    #define pLogD(frm, args...) {slogf(_SLOGC_SYNCORE, _SLOG_INFO,     "D " frm,  ##args);}
    #define pLogI(frm, args...) {slogf(_SLOGC_SYNCORE, _SLOG_INFO,     "I " frm,  ##args);}
    #define pLogW(frm, args...) {slogf(_SLOGC_SYNCORE, _SLOG_WARNING,  "W " frm, ##args);}
    #define pLogE(frm, args...) {slogf(_SLOGC_SYNCORE, _SLOG_ERROR,    "E " frm, ##args);}
    #define pLogC(frm, args...) {slogf(_SLOGC_SYNCORE, _SLOG_CRITICAL, "C " frm, ##args);}
#elif defined(__linux__)
    #ifndef CONFIG_LINUX_SYSLOG
    #define CONFIG_LINUX_SYSLOG 0
    #endif /* ifndef CONFIG_LINUX_SYSLOG */

    #if CONFIG_LINUX_SYSLOG
        #include <syslog.h>
        static  __attribute__((constructor)) void INIT_LOG()
        {
            openlog("", LOG_PID|LOG_PERROR|LOG_CONS, 0 );
        }
        #define qLogD(frm, args...) {if(dbglog::LogOutput(LOGLEVEL_L_DEBUG)) { \
            syslog(LOG_USER|LOG_DEBUG,   "D [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args); \
            if(CONFIG_LOG_STD_OUTPUT) { printf("D [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);} \
        }}
        #define qLogI(frm, args...) {if(dbglog::LogOutput(LOGLEVEL_L_INFO)) { \
            syslog(LOG_USER|LOG_INFO,    "I [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);\
            if(CONFIG_LOG_STD_OUTPUT) { printf("I [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);} \
        }}
        #define qLogW(frm, args...) {if(dbglog::LogOutput(LOGLEVEL_L_WARNING)) { \
            syslog(LOG_USER|LOG_WARNING, "W [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);\
            if(CONFIG_LOG_STD_OUTPUT) { printf("W [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);} \
        }}
        #define qLogE(frm, args...) {if(dbglog::LogOutput(LOGLEVEL_L_ERROR)) { \
            syslog(LOG_USER|LOG_ERR,     "E [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);\
            if(CONFIG_LOG_STD_OUTPUT) { printf("E [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);} \
        }}
        #define qLogC(frm, args...) {if(dbglog::LogOutput(LOGLEVEL_L_FAT)) { \
            syslog(LOG_USER|LOG_CRIT,    "C [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);\
            if(CONFIG_LOG_STD_OUTPUT) { printf("C [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);} \
        }}

        #define pLogD(frm, args...) {syslog(LOG_USER|LOG_DEBUG,   "D " frm " \n" , ##args);}
        #define pLogI(frm, args...) {syslog(LOG_USER|LOG_INFO,    "I " frm " \n" , ##args);}
        #define pLogW(frm, args...) {syslog(LOG_USER|LOG_WARNING, "W " frm " \n" , ##args);}
        #define pLogE(frm, args...) {syslog(LOG_USER|LOG_ERR,     "E " frm " \n" , ##args);}
        #define pLogC(frm, args...) {syslog(LOG_USER|LOG_CRIT,    "E " frm " \n" , ##args);}
    #else
        #define qLogD(frm, args...) {if(dbglog::LogOutput(LOGLEVEL_L_DEBUG)) {printf("D [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}
        #define qLogI(frm, args...) {if(dbglog::LogOutput(LOGLEVEL_L_INFO)) {printf("I [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}
        #define qLogW(frm, args...) {if(dbglog::LogOutput(LOGLEVEL_L_WARNING)) {printf("W [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}
        #define qLogE(frm, args...) {if(dbglog::LogOutput(LOGLEVEL_L_ERROR)) {printf("E [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}
        #define qLogC(frm, args...) {if(dbglog::LogOutput(LOGLEVEL_L_FAT)) {printf("C [%s:%d] " frm " \n" , __FUNCTION__,__LINE__,##args);}}

        #define pLogD(frm, args...) {printf("D " frm " \n" , ##args);}
        #define pLogI(frm, args...) {printf("I " frm " \n" , ##args);}
        #define pLogW(frm, args...) {printf("W " frm " \n" , ##args);}
        #define pLogE(frm, args...) {printf("E " frm " \n" , ##args);}
        #define pLogC(frm, args...) {printf("C " frm " \n" , ##args);}
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

#ifndef CONFIG_DEFAULT_LOG_LEVEL
// #define CONFIG_DEFAULT_LOG_LEVEL LOGLEVEL_L_DEBUG
#define CONFIG_DEFAULT_LOG_LEVEL LOGLEVEL_L_INFO
// #define CONFIG_DEFAULT_LOG_LEVEL LOGLEVEL_L_WARNING
// #define CONFIG_DEFAULT_LOG_LEVEL LOGLEVEL_L_ERROR
// #define CONFIG_DEFAULT_LOG_LEVEL LOGLEVEL_L_FAT
// #define CONFIG_DEFAULT_LOG_LEVEL LOGLEVEL__CLOSE
#endif

class dbglog
{
public:
    dbglog ()
    {
    }

    ~dbglog ()
    {
    }

	static int pLog_Get_cur_system_time(char *CurTimeChr, int32_t buf_size)
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

    static bool LogOutput(int lv)
    {
        if (lv <= CONFIG_DEFAULT_LOG_LEVEL) {
            return true;
        }
        return false;
    }

private:
    /* data */
};

template <size_t N = 512>
class pLog {
public:
    pLog(int _level = LOGLEVEL_L_INFO) : m_level(_level), config_level(CONFIG_DEFAULT_LOG_LEVEL), log_size(0)
    {
        std::fill(std::begin(log_message), std::end(log_message), 0);

        memset(cur_time_stamp, 0, sizeof(cur_time_stamp));
        memset(threadname, 0, sizeof(threadname));
        dbglog::pLog_Get_cur_system_time(cur_time_stamp, sizeof(cur_time_stamp));

#ifndef __QNX__     //modify by jack.wang 2022.0704 for QNX system
        get_thread_name(threadname);
#else
        std::thread::id this_id = std::this_thread::get_id();
#endif

        auto size = snprintf(log_message + log_size, N - log_size, "[%s]", cur_time_stamp);
        log_size += size;
#ifndef __QNX__     //modify by jack.wang 2022.0704 for QNX system
        size = snprintf(log_message + log_size, N - log_size, "[%s]:", threadname);
#else
        size = snprintf(log_message + log_size, N - log_size, "[T%02d]:", this_id);
#endif
        log_size += size;
    }

    ~pLog() {
        if(config_level >= m_level){
            PrintOutExitingMessage();
        }
    }

    pLog& operator << (const char* _message) {
        if(config_level >= m_level){
            if (_message == nullptr) {
                return *this;
            }

            auto str_size = strlen(_message);
            CHECK_FOR_DYNAMIC_NEED_BUFF(str_size);

            auto size = snprintf(log_message + log_size, N - log_size, "%s", _message);
            log_size += size;

            MESSAGE_OVERFLOW_FAILURE();
        }
        return *this;
    }

	pLog& operator << (std::string const& _message) {
        if(config_level >= m_level){
            *this << _message.c_str();
        }
        return *this;
    }

	pLog& operator << (int8_t const * _message) {
        if(config_level >= m_level){
            *this << (char const *)_message;
        }
        return *this;
    }

	pLog& operator << (void const * _message) {
        if(config_level >= m_level){
		    (void)_message;
        }
        return *this;
    }

	typedef std::ostream& (*ostream_manipulator)(std::ostream&);
	pLog& operator<<(ostream_manipulator pf) {
        if(config_level >= m_level){
	        (void)pf;
        }
	   return *this;
	}

    #if 0
	pLog& operator<<(std::ios_base& (*__pf) (std::ios_base&)) {
        if(config_level >= m_level){
        	(void)__pf;
    	    // _GLIBCXX_RESOLVE_LIB_DEFECTS
    	    // DR 60. What is a formatted input function?
    	    // The inserters for manipulators are *not* formatted output functions.
        }

	    return *this;
    }
    #endif

    #if 0
    pLog& operator << (nonstd::span<const uint8_t> _data) {
        if(config_level >= m_level){
            const char format_str[] = "0x%02x ";

            for (auto const& i : _data) {
                CHECK_FOR_NEED_BUFF(sizeof(format_str + 1));
                auto size = snprintf(log_message + log_size, N - log_size, format_str, i);
                log_size += size;
                // MESSAGE_OVERFLOW_FAILURE();  // T.B.D  comment out avoid for too many checks.
            }
        }
        return *this;
    }
    #endif

    pLog& operator << (char _data) {
        if(config_level >= m_level){
            CHECK_FOR_NEED_BUFF(2);

            auto size = snprintf(log_message + log_size, N - log_size, "%c", _data);
            log_size += size;

            MESSAGE_OVERFLOW_FAILURE();
        }
        return *this;
    }

    template <typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
    pLog& operator << (T _data) {
        if(config_level >= m_level){
            CHECK_FOR_NEED_BUFF(20);

            auto size = snprintf(log_message + log_size, N - log_size, "%d", static_cast<int>(_data));
            log_size += size;

            MESSAGE_OVERFLOW_FAILURE();
        }
        return *this;
    }

private:
    void PrintOutExitingMessage() {
     //   log_message[N] = 0; // for failsafe
        PrintOutByLogLevel(log_message);

        log_size = 0;
        std::fill(std::begin(log_message), std::end(log_message), 0);
    }

    void PrintOutByLogLevel(char const * _message) {
        if (_message == NULL || _message == nullptr) {
            return;
        }

        std::string LogMsg(_message);
        if (LogMsg.c_str() == NULL || LogMsg.c_str() == nullptr) {
            return;
        }
        if (strlen(LogMsg.c_str()) == 0) {
            return;
        }
        if(m_level <= LOGLEVEL_L_CLOSE){
            ;
        } else if(m_level <= LOGLEVEL_L_FAT){
            pLogC("%s", LogMsg.c_str());
        } else if(m_level <= LOGLEVEL_L_ERROR){
            pLogE("%s", LogMsg.c_str());
        } else if(m_level <= LOGLEVEL_L_WARNING){
            pLogW("%s", LogMsg.c_str());
        } else if(m_level <= LOGLEVEL_L_INFO){
            pLogI("%s", LogMsg.c_str());
        }else{
            pLogD("%s", LogMsg.c_str());
        }
    }
public:
	static bool dltLogInitFlag;

	//static DltContext CONTEXT;
private:
    char   log_message[N + 1];
    int    m_level;
	int    config_level;
    size_t log_size;
	char   threadname[32];
	char   cur_time_stamp[64];

};

#define LOG_BUFF_SIZE (512)

#define INFO    (LOGLEVEL_L_INFO)
#define WARNING (LOGLEVEL_L_WARNING)
#define ERROR   (LOGLEVEL_L_ERROR)
#define DEBUG   (LOGLEVEL_L_DEBUG)
#define FAT     (LOGLEVEL_L_FAT)
// #define LOG(x) (pLog<LOG_BUFF_SIZE>(x))
#define LOG(x) (pLog<LOG_BUFF_SIZE>(x) << "[" << __FUNCTION__ << ":" \
                << __LINE__ << "] ")


//template <size_t N>
//bool pLog<N>::dltLogInitFlag = false;
//template <size_t N>
//DltContext pLog<N>::CONTEXT;
#endif // __LIBFFWD_DBGLOG_H__
