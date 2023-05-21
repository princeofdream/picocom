/*
 * =====================================================================================
 *
 *       Filename:  netlink_event.cpp
 *
 *    Description:  netlink_event
 *
 *        Version:  1.0
 *        Created:  03/14/2019 03:55:52 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, lij1@xiaopeng.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#include <netlink_event.h>


netlink_event::netlink_event()
{
	uevent_sockfd = -1;
	linkstat = -1;
}

netlink_event::~netlink_event()
{
	if(uevent_sockfd >= 0)
		close(uevent_sockfd);
}


int
netlink_event::init_netlink_socket(int nl_groups)
{
    struct sockaddr_nl nladdr;
    int sz = 64 * MAXLEN;
    int on = 1;

	int listenfd;
	int listenlen;
	int rlen;
	char buffer[MAXLEN*4];

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    // Kernel will assign a unique nl_pid if set to zero.
    nladdr.nl_pid = 0;
	// listen link status uevent
	nladdr.nl_groups = RTMGRP_LINK|RTMGRP_IPV4_IFADDR;
	if (nl_groups != 0)
		nladdr.nl_groups = nl_groups;

    // if ((uevent_sockfd = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_KOBJECT_UEVENT)) < 0)
#ifdef COMPILE_FOR_TARGET_BY_MAKEFILE
    if ((uevent_sockfd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
#else
    if ((uevent_sockfd = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_ROUTE)) < 0)
#endif
	{
        logd("Unable to create netlink socket: %s", strerror(errno));
        return -EINVAL;
    }

    // When running in a net/user namespace, SO_RCVBUFFORCE will fail because
    // it will check for the CAP_NET_ADMIN capability in the root namespace.
    // Try using SO_RCVBUF if that fails.
    if (setsockopt(uevent_sockfd, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz)) < 0 &&
        setsockopt(uevent_sockfd, SOL_SOCKET, SO_RCVBUF, &sz, sizeof(sz)) < 0) {
        logd("Unable to set uevent socket SO_RCVBUF option: %s", strerror(errno));
        close(uevent_sockfd);
        return -EINVAL;
    }

    if (setsockopt(uevent_sockfd, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) < 0) {
        logd("Unable to set uevent socket SO_PASSCRED option: %s", strerror(errno));
        close(uevent_sockfd);
        return -EINVAL;
    }

    if (bind(uevent_sockfd, (struct sockaddr *) &nladdr, sizeof(nladdr)) < 0) {
        logd("Unable to bind netlink socket: %s", strerror(errno));
        close(uevent_sockfd);
        return -EINVAL;
    }

	return 0;
}

int
netlink_event::read_event(int (*msg_handler)(struct sockaddr_nl *, struct nlmsghdr *))
{
    int status;
    int ret = 0;
    char buf[MAXLEN*4];
    struct iovec iov = { buf, sizeof buf };
    struct sockaddr_nl snl;
    struct msghdr msg = { (void*)&snl, sizeof snl, &iov, 1, NULL, 0, 0};
    struct nlmsghdr *hmsg;

    status = recvmsg(uevent_sockfd, &msg, 0);

    if(status < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return ret;

        DBG("read_netlink: Error recvmsg: %d", status);
        loge("read_netlink: Error: ");
        return status;
    }

    if(status == 0)
        logd("read_netlink: EOF\n");

    for(hmsg = (struct nlmsghdr *) buf;
		NLMSG_OK (hmsg, (unsigned int)status);
		hmsg = NLMSG_NEXT(hmsg, status))
    {
        /* Finish reading */
        if (hmsg->nlmsg_type == NLMSG_DONE)
            return ret;

        if (hmsg->nlmsg_type == NLMSG_ERROR) {
            logd("read_netlink: Message is an error - decode TBD\n");
            return -EINVAL; // Error
        }

        /* Call message handler */
        if(msg_handler) {
            ret = (*msg_handler)(&snl, hmsg);
            if(ret < 0) {
                logd("read_netlink: Message hander error %d\n", ret);
				break;
            }
        } else {
            logd("read_netlink: Error NULL message handler\n");
            return -EINVAL;
        }
    }

    return ret;
}

int
netlink_event::netlink_link_state(struct sockaddr_nl *nl, struct nlmsghdr *msg)
{
    int len;
    struct ifinfomsg *ifi;
    char ifname[MAXLEN];
	int ret;

    ifi = (struct ifinfomsg*)NLMSG_DATA(msg);
	if_indextoname(ifi->ifi_index,ifname);

    ret = ifi->ifi_flags & IFF_RUNNING;

    logd("%s: %s LINK %s %s, ret: %#x",
		 __FUNCTION__, ifname,
		 (ifi->ifi_flags & IFF_UP)?"Up":"Down",
		 (ifi->ifi_flags & IFF_RUNNING)?"RUNNING":"",
		 ret);

    return ret;
}

int
netlink_event::msg_handler(struct sockaddr_nl *nl, struct nlmsghdr *msg)
{
    struct ifinfomsg *ifi=(struct ifinfomsg*)NLMSG_DATA(msg);
    struct ifaddrmsg *ifa=(struct ifaddrmsg*)NLMSG_DATA(msg);
    char ifname[MAXLEN];
	int ret = 0;
	if_indextoname(ifa->ifa_index,ifname);

    switch (msg->nlmsg_type)
    {
        case RTM_NEWLINK:
            logd("%s: %s RTM_NEWLINK\n", __FUNCTION__, ifname);
            break;
		case RTM_DELLINK:
            logd("%s: %s RTM_DELLINK\n", __FUNCTION__, ifname);
			break;
		case RTM_NEWADDR:
            logd("%s: %s RTM_NEWADDR\n", __FUNCTION__, ifname);
			break;
		case RTM_DELADDR:
            logd("%s: %s RTM_DELADDR\n", __FUNCTION__, ifname);
			break;
		case RTM_NEWROUTE:
            logd("%s: %s RTM_NEWROUTE\n", __FUNCTION__, ifname);
			break;
		case RTM_DELROUTE:
            logd("%s: %s RTM_DELROUTE\n", __FUNCTION__, ifname);
			break;
		case RTM_GETROUTE:
            logd("%s: %s RTM_GETROUTE\n", __FUNCTION__, ifname);
			break;
        default:
            logd("%s: %s Unknown netlink nlmsg_type %\n", __FUNCTION__,
                    ifname, msg->nlmsg_type);
            break;
    }
    return ret;
}

int
netlink_event::link_status_msg_handler(struct sockaddr_nl *nl, struct nlmsghdr *msg)
{
    struct ifinfomsg *ifi=(struct ifinfomsg*)NLMSG_DATA(msg);
    struct ifaddrmsg *ifa=(struct ifaddrmsg*)NLMSG_DATA(msg);
    char ifname[MAXLEN];
	int ret = 0;
	if_indextoname(ifa->ifa_index,ifname);

	ret = netlink_link_state(nl, msg);
	logd("msg return %#x", ret );
    return ret;
}

int
netlink_event::event_msg_handler(struct sockaddr_nl *nl, struct nlmsghdr *msg)
{
    struct ifinfomsg *ifi=(struct ifinfomsg*)NLMSG_DATA(msg);

    return msg->nlmsg_type;
}

int
netlink_event::get_uevent_sockfd()
{
	return uevent_sockfd;
}

int
netlink_event::event_handler(void* param)
{
	int ret;
	ret = read_event(msg_handler);
	return ret;
}

int
netlink_event::get_link_init_stat()
{
	return IFF_UP;
}

