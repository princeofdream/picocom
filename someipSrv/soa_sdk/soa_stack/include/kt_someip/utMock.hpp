#ifndef KT_SOMEIP_UT_MOCK_HPP
#define KT_SOMEIP_UT_MOCK_HPP
#ifdef KT_UT
#include "MockSystem.h"
#include <cstdlib>

void setCurFuncName(std::string _name);
std::string getCurFuncName();
#define MOCK0(a,b)\
if(strcmp(getCurFuncName().c_str(), __FUNCTION__) != 0 && strcmp(getCurFuncName().c_str(), "no_mock") != 0)\
{ return Mock(a##_##b).call(); }

#define MOCK1(a,b,c)\
if(strcmp(getCurFuncName().c_str(), __FUNCTION__) != 0 && strcmp(getCurFuncName().c_str(), "no_mock") != 0)\
{ return Mock(a##_##b).call(c); }

#define MOCK2(a,b,c,d)\
if(strcmp(getCurFuncName().c_str(), __FUNCTION__) != 0 && strcmp(getCurFuncName().c_str(), "no_mock") != 0)\
{ return Mock(a##_##b).call(c,d); }

#define MOCK3(a,b,c,d,e)\
if(strcmp(getCurFuncName().c_str(), __FUNCTION__) != 0 && strcmp(getCurFuncName().c_str(), "no_mock") != 0)\
{ return Mock(a##_##b).call(c,d,e); }

#define MOCK4(a,b,c,d,e,f)\
if(strcmp(getCurFuncName().c_str(), __FUNCTION__) != 0 && strcmp(getCurFuncName().c_str(), "no_mock") != 0)\
{ return Mock(a##_##b).call(c,d,e,f); }

#define MOCK5(a,b,c,d,e,f,g)\
if(strcmp(getCurFuncName().c_str(), __FUNCTION__) != 0 && strcmp(getCurFuncName().c_str(), "no_mock") != 0)\
{ return Mock(a##_##b).call(c,d,e,f,g); }

#define MOCK6(a,b,c,d,e,f,g,h)\
if(strcmp(getCurFuncName().c_str(), __FUNCTION__) != 0 && strcmp(getCurFuncName().c_str(), "no_mock") != 0)\
{ return Mock(a##_##b).call(c,d,e,f,g,h); }

#define MOCK7(a,b,c,d,e,f,g,h,i)\
if(strcmp(getCurFuncName().c_str(), __FUNCTION__) != 0 && strcmp(getCurFuncName().c_str(), "no_mock") != 0)\
{ return Mock(a##_##b).call(c,d,e,f,g,h,i); }

#define STARTTEST(name) {setCurFuncName(name);
#define ENDTEST }

#else
#define MOCK0(a,b)
#define MOCK1(a,b,c)
#define MOCK2(a,b,c,d)
#define MOCK3(a,b,c,d,e)
#define MOCK4(a,b,c,d,e,f)
#define MOCK5(a,b,c,d,e,f,g)
#define MOCK6(a,b,c,d,e,f,g,h))
#define MOCK7(a,b,c,d,e,f,g,h,i))
#endif


#endif
