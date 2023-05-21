/*
 * =====================================================================================
 *
 *       Filename:  network_filter.h
 *
 *    Description:  network_filter header
 *
 *        Version:  1.0
 *        Created:  04/03/2019 11:40:42 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#ifndef __NETWORK_FILTER_HEADER__
#define __NETWORK_FILTER_HEADER__

#include <procmgr_basic.h>
#include <arpa/inet.h>

#ifdef CONFIG_USE_NETFILTER
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>
#endif

struct network_conntrack_t {
	struct nf_conntrack *ct;
	u_int8_t proto;
	struct in_addr *local_addr;
	struct in_addr *src_addr;
	struct in_addr *dst_addr;
	u_int16_t src_port;
	u_int16_t dst_port;
#ifdef CONFIG_USE_NETFILTER
	int (*event_cb)(enum nf_conntrack_msg_type ,struct nf_conntrack *,void *);
#endif
};

class network_filter
{
public:
	network_filter ();
	virtual ~network_filter ();

#ifdef CONFIG_USE_NETFILTER
	int set_dst_addr(struct nf_conntrack *, struct in_addr*);
	int set_src_addr(struct nf_conntrack *, struct in_addr*);
	int set_proto_tcp(struct nf_conntrack *);
	int set_proto_udp(struct nf_conntrack *);
	int set_dst_port(struct nf_conntrack *, int);
	int set_src_port(struct nf_conntrack *, int);
	int set_ip_family(struct nf_conntrack *, int);

	int network_catch_connection(struct network_conntrack_t*, void*);
	int network_query_connection(struct network_conntrack_t*, void*);
#endif

private:
	/* data */
};



#endif /* ifndef __NETWORK_FILTER_HEADER__ */



