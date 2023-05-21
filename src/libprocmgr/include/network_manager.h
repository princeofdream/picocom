/*
 * =====================================================================================
 *
 *       Filename:  network_manager.h
 *
 *    Description:  network manager header
 *
 *        Version:  1.0
 *        Created:  12/17/2019 02:50:19 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin (JamesL), princeofdream@outlook.com
 *   Organization:  XPeng
 *
 * =====================================================================================
 */

#ifndef __NETWORK_MANAGER_HEADER__
#define __NETWORK_MANAGER_HEADER__

#include <procmgr_basic.h>
#include <netlink_event.h>
#include <network_utils.h>

class network_manager
{
public:
	network_manager ();
	virtual ~network_manager ();

	int start_network_manager(void* param);
	int manager_network_status(void* param);

	static int start_dhcp_service(struct nlmsghdr *msg);
	static int msg_handler(struct sockaddr_nl *nl, struct nlmsghdr *msg);
private:
};

#endif /* ifndef __NETWORK_MANAGER_HEADER__ */



