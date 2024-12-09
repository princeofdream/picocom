/**
 * @file        Handler.hpp
 * @author      Yong Li
 * @date        2022-04-27
 * @brief       Register
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
 // Authors:  Yong Li(li.yong@kotei.com.cn)                                //
 ////////////////////////////////////////////////////////////////////////////

 //---------------------------------------------------------------------------
#ifndef KT_SOMEIPLIB_HANDLER_HPP
#define KT_SOMEIPLIB_HANDLER_HPP
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <iostream>
#include <functional>
#include <map>
#include <set>
#include "kt_someip/AppSpecialMessage.hpp"

namespace kt_someip
{
    const service_t ANY_SERVICE = 0xFFFF;
    const instance_t ANY_INSTANCE = 0xFFFF;
    const method_t ANY_METHOD = 0xFFFF;
    const major_t ANY_MAJOR = 0xFF;
    const minor_t ANY_MINOR = 0xFFFFFFFF;

    const service_t DEFAULT_SERVICE = 0x0;
    const instance_t DEFAULT_INSTANCE = 0x0;
    const method_t DEFAULT_METHOD = 0x0;
    const major_t DEFAULT_MAJOR = 0x0;
    const minor_t DEFAULT_MINOR = 0x0;

    const appid_t MAX_APPID = 0xFFFF;
    const service_t MAX_SERVICE = 0xFFFF;
    const instance_t MAX_INSTANCE = 0xFFFF;
    const method_t MAX_METHOD = 0xFFFF;
    const method_t MAX_EVENTGROUP = 0xFFFF;
    const major_t MAX_MAJOR = 0xFF;
    const minor_t MAX_MINOR = 0xFFFFFFFF;

    enum class offer_type_e : uint8_t {
        OT_LOCAL = 0x00,
        OT_REMOTE = 0x01,
        OT_ALL = 0x02,
    };

    enum class subscription_type_e : uint8_t {
        SU_RELIABLE_AND_UNRELIABLE = 0x00,
        SU_PREFER_UNRELIABLE = 0x01,
        SU_PREFER_RELIABLE = 0x02,
        SU_UNRELIABLE = 0x03,
        SU_RELIABLE = 0x04,
    };

    enum class remote_state_type_e : uint8_t {
        RST_REGISTERED = 0x0,
        RST_DEREGISTERED = 0x1
    };

    enum class local_state_type_e : uint8_t {
        LST_CONNECTED = 0x0,
        LST_DISCONNECTED = 0x1
    };
    struct subEvtGrpInfo {
        /*subEvtGrpInfo(bool _is_avail, std::set<event_t> _sub_evts) {
            m_is_avail = _is_avail;
            m_sub_evts = _sub_evts;
        }*/
        bool m_is_avail;
        std::set<event_t> m_sub_evts;
    };
    typedef std::map<service_t, std::map<instance_t, 
        std::map<major_t, std::map<eventgroup_t, subEvtGrpInfo>>>> evt_avail_t;
    typedef std::function<void(const service_t, const instance_t,
        const major_t, const AppMsgEventAckCmd::e_evt_ack_type_t, eventgroup_t event_grp,
        const std::set<event_t>& events, uint64_t)> sub_evt_avail_handler_t;

    typedef std::map<service_t, std::map<instance_t, std::map<major_t, minor_t >>> available_t;

    typedef std::function<void(const remote_state_type_e)> state_handler_t;
    typedef std::function<void(const service_t, const instance_t, const major_t, const minor_t, const bool)> availability_handler_t;
    typedef std::function<void(const std::shared_ptr<AppMsgSomeipSendCmd>&)> message_handler_t;
    typedef std::function<void(const service_t, const instance_t, const event_t, bool)> offered_event_handler_t;
    typedef std::function<void(const std::vector<std::pair<service_t, instance_t>>&)> offered_services_handler_t;
    typedef std::function<bool(const appid_t, const bool)> subscription_handler_t;
    typedef std::function<void(const appid_t, const bool, std::function<void(const bool)>)> async_subscription_handler_t;
    typedef std::function<void(const service_t, const instance_t, const event_t, const eventgroup_t, const uint16_t)> subscription_status_handler_t;

    typedef std::map<service_t, std::map<instance_t, std::map<event_t, offered_event_handler_t>>> offered_event_t;

    /* Start [GAC SPEC] upload Major version / interface version error to APP*/
    typedef std::function<void(const service_t, const instance_t,
        const major_t, const major_t, const ExpMajorVersionCmd::err_type,
        const uint16_t, const std::string, uint64_t, uint16_t)> err_maj_ver_handler_t;
    /* End [GAC SPEC] upload Major version / interface version error to APP*/

    struct message_handler {
        message_handler(message_handler_t _handler) :
            handler_(_handler) {}

        bool operator<(const message_handler& _other) const {
            return handler_.target<void (*)(const std::shared_ptr<AppMsgSomeipSendCmd> &)>()
                    < _other.handler_.target<void (*)(const std::shared_ptr<AppMsgSomeipSendCmd> &)>();
        }
        message_handler_t handler_;
    };

    typedef struct _service_data {
        _service_data(service_t service, instance_t instance, major_t major, minor_t minor) {
            this->service = service;
            this->instance = instance;
            this->major = major;
            this->minor = minor;
        }

        bool operator<(const _service_data& other) const {
            return (service < other.service ||
                   (service == other.service && instance < other.instance) ||
                   (service == other.service && instance == other.instance && major < other.major));
        }

        bool operator==(const _service_data& other) const {
            return (service == other.service) && (instance == other.instance) && (major == other.major);
        }

        service_t service;
        instance_t instance;
        major_t  major;
        minor_t minor;
    }service_data;

    typedef struct _event_data {
        _event_data(service_t service, instance_t instance, major_t major, event_t event,
            std::set<eventgroup_t> eventgroups = std::set<eventgroup_t>{}) {
            this->service = service;
            this->instance = instance;
            this->major = major;
            this->event = event;
            this->eventgroups = eventgroups;
        }

        bool operator<(const _event_data& other) const {
            return (service < other.service ||
                   (service == other.service && instance < other.instance) ||
                    (service == other.service && instance == other.instance && major < other.major) ||
                   (service == other.service && instance == other.instance && major == other.major && event < other.event));
        }

        bool operator==(const _event_data& other) const {
            return (service == other.service) &&
                   (instance == other.instance) &&
                   (event == other.event) &&
                   (major == other.major);
        }

        service_t service;
        instance_t instance;
        event_t event;
        major_t major;
        std::set<eventgroup_t> eventgroups;
    }event_data;

    typedef struct _event_subscribe_data {
        _event_subscribe_data(service_t service, instance_t instance, event_t event, eventgroup_t eventGroup, major_t major) {
            this->service = service;
            this->instance = instance;
            this->event = event;
            this->eventgroup = eventGroup;
            this->major = major;
        }

        bool operator<(const _event_subscribe_data& other) const {
            return (service < other.service ||
                   (service == other.service && instance < other.instance) ||
                   (service == other.service && instance == other.instance && major < other.major) ||
                   (service == other.service && instance == other.instance && major == other.major && event < other.event));
        }

        bool operator==(const _event_subscribe_data& other) const {
            return (service == other.service) &&
                   (instance == other.instance) &&
                   (event == other.event) && 
                   (major == other.major);
        }

        service_t service;
        instance_t instance;
        event_t event;
        eventgroup_t eventgroup;
        major_t  major;
    }event_subscribe_data;
}

#endif
