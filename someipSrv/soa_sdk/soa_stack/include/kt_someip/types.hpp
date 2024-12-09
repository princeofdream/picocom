/**
 * @file        types.hpp
 * @author      Jingwei Wang
 * @date        2022-03-24
 * @brief       types
 */

////////////////////////////////////////////////////////////////////////////
//                                                                        //
// This program is the confidential and proprietary product of Kotei      //
// Any unauthorised use, reproduction or transfer of this program is      //
// strictly prohibited.                                                   //
// Copyright 2022 KOTEI. All rights reserved.                             //
//                                                                        //
// Please see the Copyright below                                         //
// This program is private software:                                      //
//                                                                        //
// Authors:  Jingwei Wang(wang.jingwei@kotei.com.cn)                      //
////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
#ifndef KT_SOMEIP_UTILS_TYPES_HPP
#define KT_SOMEIP_UTILS_TYPES_HPP
//---------------------------------------------------------------------------

#include <cstdint>
#include <string>
/* modify version string and version number for each release 
 * version number will increases by 1 each version
 */
#ifndef CONFIG_SOMEIPD_GITVER
    #define CONFIG_SOMEIPD_GITVER ""
#endif
#ifndef CONFIG_SOMEIPD_GITSTAT
    #define CONFIG_SOMEIPD_GITSTAT ""
#endif
#ifndef CONFIG_SOMEIPD_BUILD_USER
    #define CONFIG_SOMEIPD_BUILD_USER ""
#endif
#ifndef CONFIG_SOMEIPD_BUILD_HOSTNAME
    #define CONFIG_SOMEIPD_BUILD_HOSTNAME ""
#endif

#define kt_someip_ver       "1.0.001.231108.100000.ksomeip." \
    CONFIG_SOMEIPD_GITVER "" CONFIG_SOMEIPD_GITSTAT \
    ".release(" CONFIG_SOMEIPD_BUILD_USER "@" CONFIG_SOMEIPD_BUILD_HOSTNAME ")"

#define kt_someip_ver_num   (6)

#define CONFIG_MAX_PACKET   (1024 * 1024 * 20)

typedef uint8_t             BYTE;
typedef uint16_t            appid_t;
typedef uint16_t            service_t;
typedef uint16_t            instance_t;
typedef uint16_t            method_t;
typedef uint16_t            event_t;
typedef uint16_t            eventgroup_t;
typedef uint16_t            session_t;
typedef uint16_t            call_back_id;
typedef uint8_t             major_t;
typedef uint32_t            minor_t;
typedef uint16_t            client_t;
typedef uint16_t            port_num_t;

#define MAXSESSIONID  0xFF00

#ifdef _OS64
#define ptrlen              (8)
#else
#define ptrlen              (4)
#endif

struct serviceInstanceId{
    serviceInstanceId(service_t srvId = 0, instance_t instanceId = 0, major_t majr_ver = 1){
        id1 = srvId;
        id2 = instanceId;
        id3 = majr_ver;
    }

    service_t     id1; // service id
    instance_t    id2; // instance id
    major_t       id3; // major version
    bool operator < (const serviceInstanceId & p) const{
        return (id1 < p.id1) || (id1 == p.id1 && id2 < p.id2) || (id1 == p.id1 && id2 == p.id2 && id3 < p.id3);
    }

    bool operator == (const serviceInstanceId & p) const{
        return id1 == p.id1 && id2 == p.id2 && id3 == p.id3;
    }
};

struct portInfo {
    portInfo(){}
    portInfo(std::string _ip, port_num_t _port) {
        ip   = _ip;
        port = _port;
    }

    std::string ip;   // ip address
    port_num_t  port{0}; // port number
    bool operator < (const portInfo& p) const {
        return ip < p.ip ||(ip == p.ip && port < p.port);
    }

    bool operator == (const portInfo& p) const {
        return (ip == p.ip && port == p.port);
    }
};

struct eventGroupIdInfo {
    eventGroupIdInfo(uint16_t _eventGroupId, uint8_t _optIndex1, uint8_t _optIndex2) {
        eventGroupId = _eventGroupId;
        optIndex1 = _optIndex1;
        optIndex2 = _optIndex2;
    }

    uint16_t eventGroupId{ 0 };   // eventGroupId
    uint16_t optIndex1{ 0 }; // the optindex1 of current eventGroupId
    uint16_t optIndex2{ 0 }; // the optindex1 of current eventGroupId

    bool operator < (const eventGroupIdInfo& p) const {
        return (eventGroupId < p.eventGroupId) 
            || (eventGroupId == p.eventGroupId && optIndex1 < p.optIndex1) 
            || (eventGroupId == p.eventGroupId && optIndex1 == p.optIndex1 && optIndex2 < p.optIndex2);
    }

    bool operator == (const eventGroupIdInfo& p) const {
        return (eventGroupId == p.eventGroupId && optIndex1 == p.optIndex1 && optIndex2 == p.optIndex2);
    }
};

enum class ProtoType : uint8_t {
    TCP = 0x06,
    UDP = 0x11,
};

enum class e2eCheckStat : uint8_t {
    E2E_STATUS_OK = 0,
    /*E2E_STATUS_NONEWDATA
      The Check function has been invoked but
      no new Data is available since the last
      call, according to communication medium
      (e.g. RTE, COM). As a result, no E2E
      checks of Data have been consequently
      executed. This may be considered similar
      to E2E_P06STATUS_REPEATED.
    */
    E2E_STATUS_NONEWDATA = 1,
    /*E2E_STATUS_ERROR
      Error not related to counters occurred (e.g.
      wrong crc, wrong length, wrong options,
      wrong Data ID).
    */
    E2E_STATUS_ERROR = 2,
    /*E2E_STATUS_REPEATED
      The checks of the Data in this cycle were
      successful, with the exception of the repetition..
    */
    E2E_STATUS_REPEATED = 3,
    /*E2E_STATUS_OKSOMELOST
      The checks of the Data in this cycle
      were successful (including counter check,
      which was incremented within the allowed
      configured delta).
    */
    E2E_STATUS_OKSOMELOST = 4,
    /*E2E_STATUS_WRONGSEQUENCE
      The checks of the Data in this cycle were
      successful, with the exception of counter
      jump, which changed more than the allowed
      delta
    */
    E2E_STATUS_WRONGSEQUENCE = 5,
    /*E2E_OTHER_ERR
      Inner error, such as malloc error, e2e instance null error, e2e config error.
    */
    E2E_OTHER_ERR = 0XFF,
};

#endif
