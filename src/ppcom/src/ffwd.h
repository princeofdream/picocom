/*
 * =====================================================================================
 *
 *       Filename:  ffwd.h
 *
 *    Description:  Description
 *
 *        Version:  1.0
 *        Created:  2025年05月07日 15时03分39秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lijin (jin), jinli@syncore.space
 *   Organization:  SYNCORE
 *
 * =====================================================================================
 */

#ifndef __PPCOM_FFWD_H__
#define __PPCOM_FFWD_H__
#include "fifodes.h"
#include "file_descriptor.h"
#include "pipe.h"
#include "socket_server.h"
#include "socket_client.h"
#include "epoll.h"
#include "msgpak.h"
#include "dbglog.h"
#include <atomic>

class ffwd
{
public:
    ffwd ();
    virtual ~ffwd ();
    void ppcom_EpollCallbackMsg (int fd, epoll_st_t &epollSt);
    void ppcom_EpollCallbackCtl (int fd, epoll_st_t &epollSt);

    int init();
    int exit();

    int startSrv();
    int stopSrv();

    int setup_socket(const std::string& host, int port);
    void receive_messages(int sockfd);
    int send_message(int sockfd, const std::string& message);

    int getMsgFD() {
        return msgfd;
    }
    int getCtlFD() {
        return ctlfd;
    }
    void setCtlFD(int fd) {
        ctlfd = fd;
    }
    Epoll& getEpoll() {
        return mepoll;
    }
private:
    SocketServer sockServMsg;
    SocketServer sockServCtl;
    Epoll mepoll;

    int srvPort;
    int msgfd;
    int ctlfd;
    std::atomic<bool> cliRunning; // 用于控制接收线程的运行状态
    std::thread srvThrd;
};

#endif // __PPCOM_FFWD_H__

