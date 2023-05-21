/*
 * =====================================================================================
 *
 *       Filename:  network_utils.cpp
 *
 *    Description:  network utils
 *
 *        Version:  1.0
 *        Created:  12/16/2019 03:40:59 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Li Jin (JamesL), lij1@xiaopeng.com
 *   Organization:  XPeng
 *
 * =====================================================================================
 */

#include <network_utils.h>


network_utils::network_utils()
{
	pthread_mutex_init(&ifc_sock_mutex, NULL);
}

network_utils::~network_utils()
{
}

int
network_utils::configure_network(network_conf *conf)
{
	int ret;
	if (conf == NULL) {
		loge("config incorrect! abort");
		return -EINVAL;
	}

	if (strlen(conf->iface) == 0) {
		loge("interface not correct! abort!");
		return -EINVAL;
	}

	ret = ifc_configure(conf->iface, conf->ipaddr, conf->prefixlen, conf->gw);
	if (ret) {
		loge("config IP addr for %s: ip %s, prefixlen %d, gw %s, dns: %s Failed ",
			 conf->iface, conf->ipaddr, conf->prefixlen, conf->gw, conf->dns);
	}
	return ret;
}


int
network_utils::getIfaceDriver(char* ifc_name, char** ifc_driver)
{
	char iface_driver_path[MAXLEN];
	char iface_driver[MAXLEN];
	char *pp;


	memset(iface_driver_path, 0x0, sizeof(iface_driver_path));
	memset(iface_driver, 0x0, sizeof(iface_driver));

	sprintf(iface_driver_path, "/sys/class/net/%.15s/device/driver", ifc_name);
	if (readlink(iface_driver_path, iface_driver_path, sizeof(iface_driver_path))>0 && (pp = strrchr(iface_driver_path, '/'))) {
	   sprintf(iface_driver, "%s%s", iface_driver,pp+1);
	}

	if(*ifc_driver == NULL)
	{
		*ifc_driver = (char*)malloc(MAXLEN);
		memset(*ifc_driver,0x0, MAXLEN);
	}
	sprintf(*ifc_driver, "%s", iface_driver);

	return 0;
}

int
network_utils::getIfaceList(char** value)
{
    char ifaceList[MAXLEN*2];
    DIR* d;
    struct dirent* de;

    if (!(d = opendir("/sys/class/net"))) {
		loge("Cannot open iface directory");
		return -EINVAL;
    }

	if (*value == NULL) {
		*value = (char*)malloc(MAXLEN*2);
	}

	memset(ifaceList,0x0,sizeof(ifaceList));
    while ((de = readdir(d))) {
        if (de->d_name[0] == '.') continue;
		if (strlen(ifaceList) == 0)
			sprintf(ifaceList, "%s" , de->d_name);
		else
			sprintf(ifaceList, "%s %s" , ifaceList, de->d_name);
    }
    closedir(d);
	logd("ifaceList: %s", ifaceList);
	sprintf(*value, "%s", ifaceList);

    return 0;
}

int
network_utils::printIfaceList()
{
	char *ifaceList = NULL;
	char *ifacedriver = NULL;
	int ret;
	char *val;

	ret = getIfaceList(&ifaceList);

	if(ifaceList == NULL)
	{
		logd("can not get interface list!\n");
		return -EINVAL;
	}

	val = strtok(ifaceList, " ");
	while(val)
	{
		getIfaceDriver(val, &ifacedriver);
		logd("--> %s:%s <--", val, ifacedriver);
		if (ifacedriver != NULL) {
			free(ifacedriver);
			ifacedriver = NULL;
		}
		val = strtok(NULL, " ");
	}

	free(ifaceList);
	ifaceList = NULL;

    return ret;
}

int
network_utils::ifc_set_flags(const char *name, unsigned set, unsigned clr)
{
    struct ifreq ifr;
    ifc_init_ifr(name, &ifr);

    if(ioctl(ifc_ctl_sock, SIOCGIFFLAGS, &ifr) < 0) return -1;
    ifr.ifr_flags = (ifr.ifr_flags & (~clr)) | set;
    return ioctl(ifc_ctl_sock, SIOCSIFFLAGS, &ifr);
}

int
network_utils::ifc_get_flags(const char *name, struct ifreq *ifr)
{
	ifc_init_ifr(name, ifr);

	if (ioctl(ifc_ctl_sock, SIOCGIFFLAGS, ifr) != 0)
		return -EINVAL;
    return 0;
}

int
network_utils::ifc_up(const char *name)
{
    int ret = ifc_set_flags(name, IFF_UP, 0);
    logd("ifc_up(%s) = %d", name, ret);
    return ret;
}

int
network_utils::ifc_down(const char *name)
{
    int ret = ifc_set_flags(name, 0, IFF_UP);
    logd("ifc_down(%s) = %d", name, ret);
    return ret;
}

void
network_utils::init_sockaddr_in(struct sockaddr *sa, in_addr_t addr)
{
    struct sockaddr_in *sin = (struct sockaddr_in *) sa;
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    sin->sin_addr.s_addr = addr;
}

int
network_utils::ifc_set_addr(const char *name, in_addr_t addr)
{
    struct ifreq ifr;
    int ret;

    ifc_init_ifr(name, &ifr);
    init_sockaddr_in(&ifr.ifr_addr, addr);

    ret = ioctl(ifc_ctl_sock, SIOCSIFADDR, &ifr);
    logd("ifc_set_addr(%s, xx) = %d", name, ret);
    return ret;
}

void
network_utils::ifc_clear_ipv4_addresses(const char *name) {
    unsigned count, addr;
    ifc_init();
    for (count=0, addr=1;((addr != 0) && (count < 255)); count++) {
        if (ifc_get_addr(name, &addr) < 0)
            break;
        if (addr)
            ifc_set_addr(name, 0);
    }
    ifc_close();
}

int
network_utils::ifc_get_addr(const char *name, in_addr_t *addr)
{
    struct ifreq ifr;
    int ret = 0;

    ifc_init_ifr(name, &ifr);
    if (addr != NULL) {
        ret = ioctl(ifc_ctl_sock, SIOCGIFADDR, &ifr);
        if (ret < 0) {
            *addr = 0;
        } else {
            *addr = ((struct sockaddr_in*) &ifr.ifr_addr)->sin_addr.s_addr;
        }
    }
    return ret;
}

void
network_utils::ifc_init_ifr(const char *name, struct ifreq *ifr)
{
    memset(ifr, 0, sizeof(struct ifreq));
    strncpy(ifr->ifr_name, name, IFNAMSIZ);
    ifr->ifr_name[IFNAMSIZ - 1] = 0;
}

int
network_utils::ifc_init(void)
{
    int ret;

    pthread_mutex_lock(&ifc_sock_mutex);
    if (ifc_ctl_sock == -1) {
        ifc_ctl_sock = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
        if (ifc_ctl_sock < 0) {
            loge("socket() failed: %s\n", strerror(errno));
        }
    }

    ret = ifc_ctl_sock < 0 ? -1 : 0;
    logd("ifc_init_returning %d", ret);
    return ret;
}

void
network_utils::ifc_close(void)
{
    logd("ifc_close");
    if (ifc_ctl_sock != -1) {
        (void)close(ifc_ctl_sock);
        ifc_ctl_sock = -1;
    }
    pthread_mutex_unlock(&ifc_sock_mutex);
}

const char*
network_utils::ipaddr_to_string(in_addr_t addr)
{
    struct in_addr in_addr;

    in_addr.s_addr = addr;
    return inet_ntoa(in_addr);
}

int
network_utils::ifc_configure(const char *ifname, in_addr_t address,
        uint32_t prefixLength, in_addr_t gateway) {

    ifc_init();

    if (ifc_up(ifname)) {
        loge("failed to turn on interface %s: %s\n", ifname, strerror(errno));
        ifc_close();
        return -1;
    }
    if (ifc_set_addr(ifname, address)) {
        loge("failed to set ipaddr %s: %s\n", ipaddr_to_string(address), strerror(errno));
        ifc_close();
        return -1;
    }
#if 0
    if (ifc_set_prefixLength(ifname, prefixLength)) {
        loge("failed to set prefixLength %d: %s\n", prefixLength, strerror(errno));
        ifc_close();
        return -1;
    }
    if (ifc_create_default_route(ifname, gateway)) {
        loge("failed to set default route %s: %s\n", ipaddr_to_string(gateway), strerror(errno));
        ifc_close();
        return -1;
    }
#endif

    ifc_close();

    return 0;
}

