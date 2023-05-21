/*
 * =====================================================================================
 *
 *       Filename:  serv_config.h
 *
 *    Description:  serv_config header
 *
 *        Version:  1.0
 *        Created:  2022年12月17日 00时22分07秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lijin (jin), jinli@syncore.space
 *   Organization:  SYNCORE
 *
 * =====================================================================================
 */

#ifndef __SERV_CONFIG_HEADER__
#define __SERV_CONFIG_HEADER__ 1

#include "procmgr_basic.h"

#define CONFIG_DEFAULT_SERV_IP "127.0.0.1"
#define CONFIG_DEFAULT_SERV_PORT 7934

typedef struct serv_conf_t {
	char  ipaddr[CONFIG_PROCMGR_MAXLEN];
	int   port;
	int   flag;
	void* serv_cls;
	void* serv_prm;
	void* serv_ext;
} serv_conf;

class serv_config
{
public:
	serv_config ();
	virtual ~serv_config ();

	serv_conf *get_serv_conf();
	void* get_serv_source();
	void* get_serv_primary_param();
	void* get_serv_extern_param();

	void set_serv_ip(char*);
	void set_serv_port(int);
	void set_serv_flag(int);
	void set_serv_source(void*);
	void set_serv_primary_param(void*);
	void set_serv_extern_param(void*);

private:
	/* data */
	serv_conf sconf;
};


#endif /* ifndef __SERV_CONFIG_HEADER__ */


