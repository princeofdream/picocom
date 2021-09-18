/*
 * =====================================================================================
 *
 *       Filename:  debug.h
 *
 *    Description: debug
 *
 *        Version:  1.0
 *        Created:  2021年08月19日 17时06分05秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  James Lee (JamesL), princeofdream@outlook.com
 *   Organization:  BookCL
 *
 * =====================================================================================
 */

#ifndef __PPCOM_DEBUG_HEADER__
#define __PPCOM_DEBUG_HEADER__ 1

#define JDG(frm,args...) printf("--[%s:%d]--<%s>--" frm "\n",__FILE__,__LINE__,__FUNCTION__,##args)
#define JCG(frm,args...) printf("--[%s:%d]--" frm "\n",__FILE__,__LINE__,##args)



#endif /* ifndef __PPCOM_DEBUG_HEADER__ */


