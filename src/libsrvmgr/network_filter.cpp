/*
 * =====================================================================================
 *
 *       Filename:  network_filter.cpp
 *
 *    Description:  network_filter
 *
 *        Version:  1.0
 *        Created:  04/03/2019 11:39:38 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin, lij1@xiaopeng.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */

#include <network_filter.h>
#if defined(COMPILE_FOR_HOST)
#elif defined(COMPILE_FOR_TARGET_BY_MAKEFILE)
#else
#include <netutils/ifc.h>
#endif

pthread_mutex_t ip_detect_mutex;

network_filter::network_filter()
{
	pthread_mutex_init(&ip_detect_mutex, NULL);
}

network_filter::~network_filter()
{
}

#ifdef CONFIG_USE_NETFILTER
int
event_cb(enum nf_conntrack_msg_type type,struct nf_conntrack *ct,void *data)
{
        char buf[MAXLEN];
		u_int8_t get_proto;
		struct in_addr src_addr;
		struct in_addr dst_addr;
		struct in_addr get_addr;

		struct network_conntrack_t *m_nct;
		struct nf_conntrack *obj;

		if (data != NULL)
		{
			m_nct = (struct network_conntrack_t*)data;
			obj = m_nct->ct;

			if (!nfct_cmp(obj, ct, NFCT_CMP_ALL | NFCT_CMP_MASK))
				return NFCT_CB_CONTINUE;
		}

		/** Type:
		 * NFCT_T_UNKONW
		 * NFCT_T_NEW
		 * NFCT_T_DESTROY
		 * NFCF_T_UPDATE
		 * NFCT_T_ALL
		 * NFCT_T_ERROR
		 */

		get_proto = nfct_get_attr_u8(ct, ATTR_L4PROTO);
		if (get_proto == IPPROTO_ICMP)
			return NFCT_CB_CONTINUE;

		pthread_mutex_lock(&ip_detect_mutex);

		src_addr.s_addr = nfct_get_attr_u32(ct, ATTR_IPV4_SRC);
		dst_addr.s_addr = nfct_get_attr_u32(ct, ATTR_IPV4_DST);

        nfct_snprintf(buf, sizeof(buf), ct, type, NFCT_O_DEFAULT, NFCT_OF_SHOW_LAYER3);
		logd("src: %s, \t", inet_ntoa(src_addr));
		logd("dst: %s. \t ", inet_ntoa(dst_addr));
		logd("%s\n", buf);

		pthread_mutex_unlock(&ip_detect_mutex);
        return NFCT_CB_CONTINUE;
}

int
query_cb(enum nf_conntrack_msg_type type,struct nf_conntrack *ct,void *data)
{
	char buf[MAXLEN];
	u_int8_t get_proto;
	u_int32_t get_dst_addr, get_src_addr;
	struct in_addr dst_addr, src_addr;

	struct network_conntrack_t *m_nct;
	struct nf_conntrack *obj;

	if (data != NULL)
	{
		m_nct = (struct network_conntrack_t*)data;
		obj = m_nct->ct;

		if (!nfct_cmp(obj, ct, NFCT_CMP_ALL | NFCT_CMP_MASK))
			return NFCT_CB_CONTINUE;
	}

	get_proto = nfct_get_attr_u8(ct, ATTR_L4PROTO);
	get_dst_addr = nfct_get_attr_u32(ct, ATTR_REPL_IPV4_DST);
	get_src_addr = nfct_get_attr_u32(ct, ATTR_REPL_IPV4_SRC);
	dst_addr.s_addr = get_dst_addr;
	src_addr.s_addr = get_src_addr;

	if (get_proto == IPPROTO_ICMP)
		return NFCT_CB_CONTINUE;

	nfct_snprintf(buf, sizeof(buf), ct, type, NFCT_O_DEFAULT, NFCT_OF_SHOW_LAYER3);
	printf("proto=%d, dst=%s, src=%s  --- %s\n", get_proto, inet_ntoa(dst_addr), inet_ntoa(src_addr), buf);

	return NFCT_CB_CONTINUE;
}

int
network_filter::set_dst_addr(struct nf_conntrack *ct, struct in_addr *addr)
{
	if (ct == NULL || addr ==  NULL)
		return -EINVAL;

	nfct_set_attr_u32(ct, ATTR_REPL_IPV4_DST, addr->s_addr);
	return 0;
}

int
network_filter::set_src_addr(struct nf_conntrack *ct, struct in_addr *addr)
{
	if (ct == NULL || addr ==  NULL)
		return -EINVAL;

	nfct_set_attr_u32(ct, ATTR_REPL_IPV4_SRC, addr->s_addr);
	return 0;
}

int
network_filter::set_proto_tcp(struct nf_conntrack *ct)
{
	nfct_set_attr_u8(ct, ATTR_L4PROTO, IPPROTO_TCP);
	return 0;
}

int
network_filter::set_proto_udp(struct nf_conntrack *ct)
{
	nfct_set_attr_u8(ct, ATTR_L4PROTO, IPPROTO_UDP);
	return 0;
}

int
network_filter::set_dst_port(struct nf_conntrack *ct, int ip_port)
{
	nfct_set_attr_u16(ct, ATTR_REPL_PORT_DST, htons(ip_port));
	return 0;
}

int
network_filter::set_src_port(struct nf_conntrack *ct, int ip_port)
{
	nfct_set_attr_u16(ct, ATTR_REPL_PORT_SRC, htons(ip_port));
	return 0;
}

int
network_filter::set_ip_family(struct nf_conntrack *ct, int family)
{
	nfct_set_attr_u8(ct, ATTR_REPL_L3PROTO, family);
	return 0;
}

int
network_filter::network_catch_connection(struct network_conntrack_t* net_conn, void* param)
{
	int ret;
	struct nfct_handle *m_handle;
	struct nf_conntrack *ct;
	int family = AF_INET;
	int nf_flags;
	struct network_conntrack_t *m_nct = net_conn;

	if (net_conn == NULL)
		return -EINVAL;

	ct = nfct_new();

	if (!ct) {
		loge("nfct_new");
		return 0;
	}

	nf_flags = 0;
	nf_flags |= NF_NETLINK_CONNTRACK_NEW;
	nf_flags |= NF_NETLINK_CONNTRACK_DESTROY;
	// nf_flags |= NF_NETLINK_CONNTRACK_UPDATE;

	m_handle = nfct_open(CONNTRACK, nf_flags);
	if (!m_handle)
	{
		loge("catch nfct_open");
		nfct_destroy(ct);
		return -EINVAL;
	}


	set_ip_family(ct, family);
	if (m_nct->src_addr != NULL)
		set_src_addr(ct, m_nct->src_addr);

	if (m_nct->dst_addr != NULL)
		set_dst_addr(ct, m_nct->dst_addr);

	m_nct->ct = ct;

	set_proto_tcp(ct);
	if (net_conn->event_cb == NULL)
	{
		logd("Use internal event cb!\n");
		net_conn->event_cb = event_cb;
	}
	nfct_callback_register(m_handle, NFCT_T_ALL, net_conn->event_cb, m_nct);
	ret = nfct_catch(m_handle);

	if (ret == -1)
			logd("%s: (%d)(%s)\n", __func__, ret, strerror(errno));
	else
			logd("%s: (OK)\n",__func__);

	nfct_close(m_handle);
	nfct_destroy(ct);

	return 0;
}

int
network_filter::network_query_connection(struct network_conntrack_t* net_conn, void* param)
{
        int ret;
        struct nfct_handle *m_handle;
        struct nf_conntrack *ct;
		int family = AF_INET;
		int nf_flags;
		struct network_conntrack_t *m_nct = net_conn;

		if (net_conn == NULL)
			return -EINVAL;

        ct = nfct_new();

        if (!ct) {
			// loge("nfct_new");
			return 0;
        }

		nf_flags = 0;

        m_handle = nfct_open(CONNTRACK, nf_flags);
        if (!m_handle)
		{
			loge("query nfct_open");
			nfct_destroy(ct);
			return -EINVAL;
		}

		set_ip_family(ct, family);
		if (m_nct->src_addr != NULL)
			set_src_addr(ct, m_nct->src_addr);

		if (m_nct->dst_addr != NULL)
			set_dst_addr(ct, m_nct->dst_addr);

		m_nct->ct = ct;

		// set_proto_tcp(ct);
		// set_proto_udp(ct);
		if (m_nct->event_cb == NULL)
		{
			logd("Use internal query cb!\n");
			m_nct->event_cb = query_cb;
		}
		nfct_callback_register(m_handle, NFCT_T_ALL, m_nct->event_cb, m_nct);
		ret = nfct_query(m_handle, NFCT_Q_DUMP, &family);

#if 0
        if (ret == -1)
			logd("%s: (%d)(%s)\n", __func__, ret, strerror(errno));
        else
			logd("%s: (OK)\n", __func__);
#endif

        nfct_close(m_handle);
        nfct_destroy(ct);

        return 0;
}

#endif

