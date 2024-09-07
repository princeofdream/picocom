/*
 * =====================================================================================
 *
 *       Filename:  ctrl_term.h
 *
 *    Description:  ctrl_term header
 *
 *        Version:  1.0
 *        Created:  2021年09月18日 13时50分15秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  James Lee (JamesL), princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */


#ifndef __LOG_SERVER_HEADER__
#define __LOG_SERVER_HEADER__ 1


#if 1 //def CONFIG_LIB_SRVMGR

#include <service_manager.h>
#include <procmgr_basic.h>

#define READ_BUFFER_LEN 4096

void start_ctrl_term(int infd, int outfd);
void stop_ctrl_term(void* param);




#endif


#endif /* ifndef __LOG_SERVER_HEADER__ */

