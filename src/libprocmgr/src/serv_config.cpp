/*
 * =====================================================================================
 *
 *       Filename:  serv_config.cpp
 *
 *    Description:  serv_config
 *
 *        Version:  1.0
 *        Created:  2022年12月17日 00时31分42秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lijin (jin), jinli@syncore.space
 *   Organization:  SYNCORE
 *
 * =====================================================================================
 */

#include <serv_config.h>

serv_config::serv_config()
{
}

serv_config::~serv_config()
{
}

serv_conf*
serv_config::get_serv_conf()
{
	return &sconf;
}

void*
serv_config::get_serv_source()
{
	return sconf.serv_cls;
}

void*
serv_config::get_serv_primary_param()
{
	return sconf.serv_prm;
}

void*
serv_config::get_serv_extern_param()
{
	return sconf.serv_ext;
}

void
serv_config::set_serv_ip(char* ipaddr)
{
	memset(sconf.ipaddr, 0x0, sizeof(ipaddr));
	sprintf(sconf.ipaddr, "%s", ipaddr);
}

void
serv_config::set_serv_port(int val)
{
	sconf.port = val;
}

void
serv_config::set_serv_flag(int val)
{
	sconf.flag = val;
}

void
serv_config::set_serv_source(void* param)
{
	sconf.serv_cls = param;
}

void
serv_config::set_serv_primary_param(void* param)
{
	sconf.serv_prm = param;
}

void
serv_config::set_serv_extern_param(void* param)
{
	sconf.serv_ext = param;
}




