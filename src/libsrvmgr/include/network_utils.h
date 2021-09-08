/*
 * =====================================================================================
 *
 *       Filename:  network_utils.h
 *
 *    Description:  network_utils header
 *
 *        Version:  1.0
 *        Created:  12/16/2019 03:41:21 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin (JamesL), lij1@xiaopeng.com
 *   Organization:  XPeng
 *
 * =====================================================================================
 */


#ifndef __NETWORK_UTILS_HEADER__
#define __NETWORK_UTILS_HEADER__

#include <basic.h>
#include <process_manager.h>

typedef struct network_conf_t {
	char iface[128];
	in_addr_t ipaddr;
	in_addr_t gw;
	in_addr_t dns;
	int prefixlen;
} network_conf;

class network_utils
{
public:
	network_utils ();
	virtual ~network_utils ();

	int network_start_dhcp_server_utils();
	int configure_network(network_conf *conf);

	int getIfaceDriver(char*, char**);
	int getIfaceList(char**);
	int printIfaceList();

	int ifc_up(const char *name);
	int ifc_down(const char *name);
	int ifc_set_addr(const char *name, in_addr_t addr);
	void ifc_clear_ipv4_addresses(const char *name);
	int ifc_get_addr(const char *name, in_addr_t *addr);
	int ifc_init(void);
	void ifc_close(void);
	int ifc_configure(const char *ifname, in_addr_t address,
        uint32_t prefixLength, in_addr_t gateway);
	const char *ipaddr_to_string(in_addr_t addr);
	int ifc_get_flags(const char *name, struct ifreq *ifr);

private:
	void init_sockaddr_in(struct sockaddr *sa, in_addr_t addr);
	int ifc_set_flags(const char *name, unsigned set, unsigned clr);
	void ifc_init_ifr(const char *name, struct ifreq *ifr);

private:
	pthread_mutex_t ifc_sock_mutex;
	int ifc_ctl_sock = -1;

};

#endif /* ifndef __NETWORK_UTILS_HEADER__ */


