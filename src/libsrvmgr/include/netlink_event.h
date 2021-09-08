/*
 * =====================================================================================
 *
 *       Filename:  netlink_event.h
 *
 *    Description:  local uevent header
 *
 *        Version:  1.0
 *        Created:  03/14/2019 03:53:58 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, lij1@xiaopeng.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#ifndef __NETLINK_EVENT_HEADER__
#define __NETLINK_EVENT_HEADER__

#include <basic.h>

class netlink_event
{
public:
	netlink_event ();
	virtual ~netlink_event ();

	int init_netlink_socket(int);
	int read_event(int (*handler)(struct sockaddr_nl *,struct nlmsghdr *));

	static int netlink_link_state(struct sockaddr_nl *, struct nlmsghdr *);
	static int msg_handler(struct sockaddr_nl *, struct nlmsghdr *);
	static int link_status_msg_handler(struct sockaddr_nl *nl, struct nlmsghdr *msg);
	static int event_msg_handler(struct sockaddr_nl *, struct nlmsghdr *);

	int get_uevent_sockfd();
	int get_link_init_stat();

	int event_handler(void*);

private:
	int uevent_sockfd;
	int linkstat;
};

#endif /* ifndef __NETLINK_EVENT_HEADER__ */

