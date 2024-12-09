/*
 * =====================================================================================
 *
 *       Filename:  Intercomm.h
 *
 *    Description:  Description
 *
 *        Version:  1.0
 *        Created:  2024年12月09日 10时23分28秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lijin (jin), jinli@syncore.space
 *   Organization:  SYNCORE
 *
 * =====================================================================================
 */

#ifndef INTERCOM_COMM_HEADER__
#define INTERCOM_COMM_HEADER__ 1

#include <iostream>
#include <random>
#include <mutex>
#ifdef __QNX__
#include <sys/pps.h>
#endif
#include "InterCommunication_serviceimpl.hpp"


using namespace v1::ara_api;

class IntercommSrv
{
public:
    IntercommSrv ();
    virtual ~IntercommSrv ();

    int Init();
    int MainThread(int val);
    int startSrvThread();

private:
    std::unique_ptr<InterCommunication::skeleton::InterCommunicationSkeleton>
        IntercommSkeleton;
    std::thread srvThread;
    int srvReady;

    InterCommunication::skeleton::fields::InterField::FieldType g_InterField;
    InterCommunication::skeleton::fields::InterFieldArr::FieldType g_InterFieldArr;

    std::mutex msgMtx;
    std::timed_mutex tmMtx;
};

#endif /* ifndef INTERCOM_COMM_HEADER__ */

