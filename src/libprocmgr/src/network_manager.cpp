/*
 * =====================================================================================
 *
 *       Filename:  network_manager.cpp
 *
 *    Description:  network manager
 *
 *        Version:  1.0
 *        Created:  12/17/2019 02:49:58 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin (JamesL), princeofdream@outlook.com
 *   Organization:  XPeng
 *
 * =====================================================================================
 */

#include <network_manager.h>
// #include <dnsmasq_utils.h>
#include <network_utils.h>

struct ifreq wlan_ifr;

network_manager::network_manager()
{
	char *iface_list = NULL;
	char *iface = NULL;
	static network_utils m_net_util;

	m_net_util.getIfaceList(&iface_list);
	plogi("iface list: \n%s\n", iface_list);
	if (iface_list == NULL) {
		return;
	}

	iface = strtok(iface_list, " ");
	while (true) {
		if (iface == NULL)
			break;

		iface = strtok(NULL, " ");
	}

	free(iface_list);
}

network_manager::~network_manager()
{
}

int
network_manager::start_network_manager(void* param)
{
	manager_network_status(param);
	return 0;
}

int
network_manager::manager_network_status(void* param)
{
	int msockfd = -1;
	int fd_event;
	int i0;
	int rlen;
	int wlen;
	char buffer[CONFIG_PROCMGR_BUFFER_LEN];
	int fd_max;
	fd_set rd_set;
	int mlink;
	int sel_ret;
	int nl_groups;
	proc_conf *mparam = (proc_conf*)param;

	netlink_event mevent;

	if (mparam == NULL) {
		ploge("param is NULL!");
		// return -EINVAL;
	}

	nl_groups = RTMGRP_LINK;
	mevent.init_netlink_socket(nl_groups);
	fd_event = mevent.get_uevent_sockfd();

	msockfd = STDIN_FILENO;
	fd_max = PROCMGR_MAX_VAL(msockfd, fd_event) + 1;

	plogd("fd_event: %d, msockfd: %d, fd_max: %d", fd_event, msockfd, fd_max);

	if (mparam != NULL) {
		if ((mparam->flag & FLAG_SYNC_MUTEX) == FLAG_SYNC_MUTEX) {
			plogd("thread unlock.");
			pthread_mutex_unlock(mparam->mlock);
		}
	}

	i0 = 0;
	while(true) {
		FD_ZERO(&rd_set);

		FD_SET(msockfd  , &rd_set);
		FD_SET(fd_event , &rd_set);

		// set timeval NULL to block
		sel_ret = select(fd_max, &rd_set, NULL, NULL , NULL);
		if (sel_ret < 0 ) {
			plogd();
			return -EINVAL;
		}

		if (FD_ISSET(msockfd, &rd_set))
		{
			memset(buffer,0x0,sizeof(buffer));
			rlen = read(msockfd, buffer, sizeof(buffer) - 1);
			plogd("get msg %d, len:[%d] : %s", i0, rlen, buffer);

			if(rlen <= 0 && sel_ret == 1) {
				plogd("socket closed!, rlen: %d", rlen);
				break;
			}

			sprintf(buffer, "%s OK", buffer);
			if (msockfd == STDIN_FILENO)
				wlen = write(STDOUT_FILENO, buffer, strlen(buffer) + 1);
			else
				wlen = write(msockfd, buffer, strlen(buffer) + 1);
			plogd("send msg<%d>, len:[%d] : %s", i0, wlen, buffer);

		}
		else if (FD_ISSET(fd_event, &rd_set))
		{
			memset(buffer,0x0,sizeof(buffer));
			mlink = mevent.read_event(msg_handler);

			if (msockfd == STDIN_FILENO)
				wlen = write(STDOUT_FILENO, buffer, strlen(buffer) + 1);
			else
				wlen = write(msockfd, buffer, strlen(buffer) + 1);
			plogd("send msg %d, len:[%d] : %s", i0, wlen, buffer);
			continue;
		}

		if(rlen <= 0 && sel_ret == 1)
		{
			plogd("socket closed!, rlen: %d", rlen);
			break;
		}
		i0++;
	}
	FD_ZERO(&rd_set);
	return 0;
}

int
network_manager::start_dhcp_service(struct nlmsghdr *msg)
{
    int len;
    struct ifinfomsg *ifi;
    char ifname[CONFIG_PROCMGR_MAXLEN];
	int ret;

    ifi = (struct ifinfomsg*)NLMSG_DATA(msg);
	if_indextoname(ifi->ifi_index,ifname);

    ret = ifi->ifi_flags & IFF_RUNNING;

    plogd("pre stat: %s: %s LINK %s %s, ret: %#x", __FUNCTION__, wlan_ifr.ifr_name,
		 (wlan_ifr.ifr_flags & IFF_UP)?"Up":"Down",
		 (wlan_ifr.ifr_flags & IFF_RUNNING)?"RUNNING":"",
		 wlan_ifr.ifr_flags);
    plogd("current stat: %s: %s LINK %s %s, ret: %#x", __FUNCTION__, ifname,
		 (ifi->ifi_flags & IFF_UP)?"Up":"Down",
		 (ifi->ifi_flags & IFF_RUNNING)?"RUNNING":"",
		 ifi->ifi_flags);


	// TODO
	// Start dnsmasq in process startup and run in background will be fine
	// Or if we want to start server by listen to wlan0 stat, enable this
#if 0
	if (((wlan_ifr.ifr_flags & IFF_RUNNING) != IFF_RUNNING)
		&& ((ifi->ifi_flags & IFF_RUNNING) == IFF_RUNNING)) {
		plogd("Start dhcp server");
		process_manager dnsmasq_proc;
		proc_conf mp_dnsmasq_param;

		mp_dnsmasq_param.flag        = FLAG_DEFAULT;
		mp_dnsmasq_param.proc_param  = NULL;

		dnsmasq_proc.start_routine = start_dnsmasq_utils_proc;
		dnsmasq_proc.start_process((void*)&mp_dnsmasq_param);
		plogd("start dnsmasq service done! \n");
	} else if (((wlan_ifr.ifr_flags & IFF_RUNNING) == IFF_RUNNING)
		&& ((ifi->ifi_flags & IFF_RUNNING) != IFF_RUNNING)) {
		plogd("Stop dhcp server");
		// stop_dnsmasq_utils_proc(NULL);
	}
#endif

	wlan_ifr.ifr_flags = ifi->ifi_flags;

    return ret;
}

int
network_manager::msg_handler(struct sockaddr_nl *nl, struct nlmsghdr *msg)
{
    struct ifinfomsg *ifi=(struct ifinfomsg*)NLMSG_DATA(msg);
    struct ifaddrmsg *ifa=(struct ifaddrmsg*)NLMSG_DATA(msg);
    char ifname[CONFIG_PROCMGR_MAXLEN];
	int ret = 0;

	if_indextoname(ifa->ifa_index, ifname);

#if 0
	if (strcmp(ifname, "eth0") != 0) {
		plogd("iface: %s, not hostapd iface, abort!", ifname);
		return 0;
	}

    ifi = (struct ifinfomsg*)NLMSG_DATA(msg);
    switch (msg->nlmsg_type)
    {
        case RTM_NEWLINK:
            plogd("%s: %s RTM_NEWLINK\n", __FUNCTION__, ifname);
			start_dhcp_service(msg);
            break;
		case RTM_DELLINK:
            plogd("%s: %s RTM_DELLINK\n", __FUNCTION__, ifname);
			break;
		case RTM_NEWADDR:
            plogd("%s: %s RTM_NEWADDR\n", __FUNCTION__, ifname);
			break;
		case RTM_DELADDR:
            plogd("%s: %s RTM_DELADDR\n", __FUNCTION__, ifname);
			break;
		case RTM_NEWROUTE:
            plogd("%s: %s RTM_NEWROUTE\n", __FUNCTION__, ifname);
			break;
		case RTM_DELROUTE:
            plogd("%s: %s RTM_DELROUTE\n", __FUNCTION__, ifname);
			break;
		case RTM_GETROUTE:
            plogd("%s: %s RTM_GETROUTE\n", __FUNCTION__, ifname);
			break;
        default:
            plogd("%s: %s Unknown netlink nlmsg_type %\n", __FUNCTION__,
                    ifname, msg->nlmsg_type);
            break;
    }
#endif
    return ret;
}







