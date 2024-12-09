/**
 * @file        AppSpecialMessage.hpp
 * @author      Jingwei Wang
 * @date        2022-04-06
 * @brief       AppSpecialMessage
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
#ifndef KT_SOMEIP_APPSPECIALMESSAGE_HPP
#define KT_SOMEIP_APPSPECIALMESSAGE_HPP
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <vector>
#include <set>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include "kt_someip/AppMessage.hpp"
#include "kt_someip/someipMessage/someip_message.h"

namespace kt_someip
{
    /* payload struct for below command
       SOMEIP_REQUEST_APPID
       SOMEIP_DEREGISTER_APPLICATION */
    class AppMsgAppId : public AppMessage {
    public:
        AppMsgAppId(uint8_t _cmd_id) : AppMessage(APP_HEADER_LEN) {
            setCommandId(_cmd_id);
        }
        void unserializePayload() {
            // no payload
        }
        ~AppMsgAppId() {}
        uint16_t getSrvid() { return 0; }
        uint16_t getInsid() { return 0; }
        uint8_t getMajor() { return 0; }
		std::string getCmdString() { return "SOMEIP_REQUEST_APPID"; };

    };

    /* SOMEIP_PING */
    class AppMsgPing : public AppMessage {
    public:
        AppMsgPing() :AppMessage(APP_HEADER_LEN) {
            setCommandId(SOMEIP_PING);
            setReseved(kt_someip_ver_num);
        }
        void unserializePayload() {
        }
        ~AppMsgPing() {}
        uint16_t getSrvid() { return 0; }
        uint16_t getInsid() { return 0; }
        uint8_t getMajor() { return 0; }
		std::string getCmdString() { return "SOMEIP_PING"; };
    };

    /* SOMEIP_PONG */
    class AppMsgPong : public AppMessage {
    public:
        AppMsgPong() :AppMessage(APP_HEADER_LEN) {
            setCommandId(SOMEIP_PONG);
            setReseved(kt_someip_ver_num);
        }
        void unserializePayload() {
        }
        ~AppMsgPong() {}
        uint16_t getSrvid() { return 0; }
        uint16_t getInsid() { return 0; }
        uint8_t getMajor() { return 0; }
		std::string getCmdString() { return "SOMEIP_PONG"; };
    };

    /* payload struct for below command
        SOMEIP_REQUEST_APPID_ACK */
    class AppMsgRequestAppIdAckData : public AppMessage {
    public:
        AppMsgRequestAppIdAckData(uint16_t _appid, uint8_t _log_lev,
            /*Start [GAC SPEC] service exception check */
            uint32_t _server_time = 10000, uint32_t _cli_time = 10000, bool _console_log = false) :
            AppMessage(REQ_APP_ID_ACK_LEN),
            /*End [GAC SPEC] service exception check */
            m_appid(_appid),
            m_log_level(_log_lev),
            /*Start [GAC SPEC] service exception check */
            m_server_exp_time(_server_time),
            m_client_exp_time(_cli_time),
            /*End [GAC SPEC] service exception check */
            m_console_log(_console_log){
            setCommandId(SOMEIP_REQUEST_APPID_ACK);
            memcpy(&m_data[PAYLOAD_POS], &m_appid, sizeof(m_appid));
            memcpy(&m_data[PAYLOAD_POS + 2], &m_log_level, sizeof(m_log_level));
            memcpy(&m_data[PAYLOAD_POS + 3], &m_server_exp_time, sizeof(m_server_exp_time));
            memcpy(&m_data[PAYLOAD_POS + 7], &m_client_exp_time, sizeof(m_client_exp_time));
            memcpy(&m_data[PAYLOAD_POS + 11], &m_console_log, sizeof(m_console_log));
        }
        AppMsgRequestAppIdAckData() :AppMessage(REQ_APP_ID_ACK_LEN) {
            setCommandId(SOMEIP_REQUEST_APPID_ACK);
        }

        ~AppMsgRequestAppIdAckData() {}
        void unserializePayload() {
            memcpy(&m_appid, &m_data[PAYLOAD_POS], sizeof(m_appid));
            memcpy(&m_log_level, &m_data[PAYLOAD_POS + 2], sizeof(m_log_level));
            memcpy(&m_server_exp_time, &m_data[PAYLOAD_POS + 3], sizeof(m_server_exp_time));
            memcpy(&m_client_exp_time, &m_data[PAYLOAD_POS + 7], sizeof(m_client_exp_time));
            memcpy(&m_console_log, &m_data[PAYLOAD_POS + 11], sizeof(m_console_log));
        }
        void setAppid(uint16_t _id) {
            m_appid = _id;
            memcpy(&m_data[PAYLOAD_POS], &m_appid, sizeof(m_appid));
        }
        uint16_t getAppid() { return m_appid; }
        uint8_t getLogLevel() { return m_log_level; }
        uint16_t getSrvid() { return 0; }
        uint16_t getInsid() { return 0; }
        uint8_t getMajor() { return 0; }
		std::string getCmdString() {
            std::string str = "SOMEIP_REQUEST_APPID_ACK, appid:"
                + std::to_string(m_appid) + ", loglevel:"
                + std::to_string(m_log_level);
            return str;
        };
        /*Start [GAC SPEC] service exception check */
        uint32_t getServerExpTime() { return m_server_exp_time; }
        uint32_t getClientExpTime() { return m_client_exp_time; }
        /*End [GAC SPEC] service exception check */
        bool getConsoleLog() { return m_console_log; }

    private:
        uint16_t    m_appid{0};
        uint8_t     m_log_level{0};
        /*Start [GAC SPEC] service exception check */
        uint32_t    m_server_exp_time{ 10000 }; // : ms
        uint32_t    m_client_exp_time{ 10000 }; // : ms
        /*End [GAC SPEC] service exception check */
        bool        m_console_log{ false };
    };

    /* SOMEIP_UPPERTESTERCMD 
    * Change all service's m_cyclic_offer_delay and ttl
    */
    class AppMsgUpperTerter : public AppMessage {
    public:
        AppMsgUpperTerter() :AppMessage(APP_HEADER_LEN + 10) {
            setCommandId(SOMEIP_UPPERTESTERCMD);
        }
        AppMsgUpperTerter(uint16_t _cmd, uint32_t _dly, uint32_t _ttl) 
            :AppMessage(APP_HEADER_LEN + 10) {
            setCommandId(SOMEIP_UPPERTESTERCMD);
            m_test_cmd = _cmd;
            m_sd_cyc_offer_sub_dly = _dly;
            m_sd_offer_ttl = _ttl;
            memcpy(&m_data[PAYLOAD_POS], &m_test_cmd, sizeof(m_test_cmd));
            memcpy(&m_data[PAYLOAD_POS + 2], &m_sd_cyc_offer_sub_dly, sizeof(m_sd_cyc_offer_sub_dly));
            memcpy(&m_data[PAYLOAD_POS + 6], &m_sd_offer_ttl, sizeof(m_sd_offer_ttl));
        }
        void unserializePayload() {
            memcpy(&m_test_cmd, &m_data[PAYLOAD_POS], sizeof(m_test_cmd));
            memcpy(&m_sd_cyc_offer_sub_dly, &m_data[PAYLOAD_POS + 2], sizeof(m_sd_cyc_offer_sub_dly));
            memcpy(&m_sd_offer_ttl, &m_data[PAYLOAD_POS + 6], sizeof(m_sd_offer_ttl));
        }
        ~AppMsgUpperTerter() {}
        void setCmd(uint16_t _cmd) {
            m_test_cmd = _cmd;
            memcpy(&m_data[PAYLOAD_POS], &m_test_cmd, sizeof(m_test_cmd));
        }
        void setDly(uint32_t _dly) {
            m_sd_cyc_offer_sub_dly = _dly;
            memcpy(&m_data[PAYLOAD_POS + 2], &m_sd_cyc_offer_sub_dly, sizeof(m_sd_cyc_offer_sub_dly));
        }
        void setTtl(uint32_t _ttl) {
            m_sd_offer_ttl = _ttl;
            memcpy(&m_data[PAYLOAD_POS + 6], &m_sd_offer_ttl, sizeof(m_sd_offer_ttl));
        }
        uint16_t getTestCmd() { return m_test_cmd; }
        uint32_t getDly() { return m_sd_cyc_offer_sub_dly; }
        uint32_t getTtl() { return m_sd_offer_ttl; }
        uint16_t getSrvid() { return 0; }
        uint16_t getInsid() { return 0; }
        uint8_t getMajor() { return 0; }
        std::string getCmdString() { return "SOMEIP_UPPERTESTERCMD"; };
    private:
        uint16_t    m_test_cmd{0}; // 1: end test 0x32: clientServiceSubscribeEventgroup // 0xfe: print trace
        uint32_t    m_sd_cyc_offer_sub_dly{0};
        uint32_t    m_sd_offer_ttl{ 0 };
    };

    /* payload struct for below command
        SOMEIP_OFFER_SERVICE
        SOMEIP_STOP_OFFER_SERVICE
        SOMEIP_RELEASE_SERVICE
        SOMEIP_REQUEST_SERVICE */
    class AppMsgServiceCmd : public AppMessage {
    public:
        AppMsgServiceCmd(uint8_t _cmd_id, uint16_t _srv_id, uint16_t _ins_id, 
            uint8_t _major = 0, uint32_t _minor = 0) :
            AppMessage(APP_SERVICE_MSG_LEN),
            m_srv_id(_srv_id),
            m_ins_id(_ins_id),
            m_major(_major),
            m_minor(_minor) {
            setCommandId(_cmd_id);
            memcpy(&m_data[PAYLOAD_POS], &_srv_id, sizeof(_srv_id));
            memcpy(&m_data[PAYLOAD_POS + 2], &_ins_id, sizeof(_ins_id));
            memcpy(&m_data[PAYLOAD_POS + 4], &_major, sizeof(_major));
            memcpy(&m_data[PAYLOAD_POS + 5], &_minor, sizeof(_minor));
        }
        AppMsgServiceCmd(uint8_t _cmd_id) :AppMessage(APP_SERVICE_MSG_LEN) {
            setCommandId(_cmd_id);
        }

        ~AppMsgServiceCmd() {}
        void unserializePayload() {
            memcpy(&m_srv_id, &m_data[PAYLOAD_POS], sizeof(m_srv_id));
            memcpy(&m_ins_id, &m_data[PAYLOAD_POS + 2], sizeof(m_ins_id));
            memcpy(&m_major, &m_data[PAYLOAD_POS + 4], sizeof(m_major));
            memcpy(&m_minor, &m_data[PAYLOAD_POS + 5], sizeof(m_minor));
        }

        void setSrvid(uint16_t _id) { 
            m_srv_id = _id;
            memcpy(&m_data[PAYLOAD_POS], &m_srv_id, sizeof(m_srv_id));
        }
        uint16_t getSrvid() { return m_srv_id; }

        void setInsid(uint16_t _id) { 
            m_ins_id = _id;
            memcpy(&m_data[PAYLOAD_POS + 2], &m_ins_id, sizeof(m_ins_id));
        }
        uint16_t getInsid() { return m_ins_id; }

        void setMajor(uint8_t _id) {
            m_major = _id;
            memcpy(&m_data[PAYLOAD_POS + 4], &m_major, sizeof(m_major));
        }
        uint8_t getMajor() { return m_major; }

        void setMinor(uint32_t _id) {
            m_minor = _id;
            memcpy(&m_data[PAYLOAD_POS + 5], &m_minor, sizeof(m_minor));
        }
        uint32_t getMinor() { return m_minor; }
		std::string getCmdString() { 
            std::map<uint8_t, std::string> cmd_name = {
                {SOMEIP_OFFER_SERVICE, "SOMEIP_OFFER_SERVICE"},
                {SOMEIP_STOP_OFFER_SERVICE, "SOMEIP_STOP_OFFER_SERVICE"},
                {SOMEIP_RELEASE_SERVICE, "SOMEIP_RELEASE_SERVICE"},
                {SOMEIP_REQUEST_SERVICE, "SOMEIP_REQUEST_SERVICE"},
            };
            std::string str = cmd_name[getCommandId()] + "("
                + std::to_string(m_srv_id) + ","
                + std::to_string(m_ins_id) + ","
                + std::to_string(m_major) + ","
                + std::to_string(m_minor) + ")";
            return str;
        };

    private:
        uint16_t    m_srv_id;
        uint16_t    m_ins_id;
        uint8_t     m_major;
        uint32_t    m_minor;
    };
    /* SOMEIP_REQUEST_EVENT_ACK */
    class AppMsgEventAckCmd : public AppMessage {
        const uint16_t LEN = sizeof(eventgroup_t);
    public:
        enum class e_evt_ack_type_t: uint8_t {
            E_AVAIL = 1,
            E_EXCEPTION = 2, // lost ACK 10 times exception
            E_LOST_ONCE = 3  // lost ACK 1 time
        };
        AppMsgEventAckCmd(uint16_t _srv_id, uint16_t _ins_id, uint8_t _maj_ver, e_evt_ack_type_t _is_avail,
            eventgroup_t _event_grp, std::set<event_t>& _events) :
            AppMessage(APP_HEADER_LEN),
            m_srv_id(_srv_id),
            m_ins_id(_ins_id),
            m_major(_maj_ver),
            m_is_avail(_is_avail),
            m_event_grp(_event_grp),
            m_events(_events) {
            size_t size = _events.size();
            setMessageLen(APP_HEADER_LEN + 16 + size * LEN);
            auto now = std::chrono::system_clock::now();
            m_timestamp = std::chrono::duration_cast<
                std::chrono::milliseconds>(now.time_since_epoch()).count();
            memcpy(&m_data[PAYLOAD_POS], &m_timestamp, sizeof(m_timestamp));
            setCommandId(SOMEIP_REQUEST_EVENT_ACK);
            memcpy(&m_data[PAYLOAD_POS + 8], &_srv_id, sizeof(_srv_id));
            memcpy(&m_data[PAYLOAD_POS + 10], &_ins_id, sizeof(_ins_id));
            memcpy(&m_data[PAYLOAD_POS + 12], &_maj_ver, sizeof(_maj_ver));
            memcpy(&m_data[PAYLOAD_POS + 13], &_is_avail, sizeof(_is_avail));
            memcpy(&m_data[PAYLOAD_POS + 14], &_event_grp, sizeof(_event_grp));
            uint16_t i = 0;
            for (auto eid : _events) {
                memcpy(&m_data[PAYLOAD_POS + 16 + i * LEN], &eid, sizeof(event_t));
                ++i;
            }
        }

        AppMsgEventAckCmd(const size_t _in_len) :AppMessage(_in_len) {
            setCommandId(SOMEIP_REQUEST_EVENT_ACK);
        }

        ~AppMsgEventAckCmd() {}
        void unserializePayload() {
            if (m_data.size() < (APP_HEADER_LEN + 16)) {
                return;
            }
            memcpy(&m_timestamp, &m_data[PAYLOAD_POS], sizeof(m_timestamp));
            memcpy(&m_srv_id, &m_data[PAYLOAD_POS + 8], sizeof(m_srv_id));
            memcpy(&m_ins_id, &m_data[PAYLOAD_POS + 10], sizeof(m_ins_id));
            memcpy(&m_major, &m_data[PAYLOAD_POS + 12], sizeof(m_major));
            uint8_t avail_flag = 0;
            memcpy(&avail_flag, &m_data[PAYLOAD_POS + 13], sizeof(avail_flag));
            m_is_avail = static_cast<e_evt_ack_type_t>(avail_flag);
            memcpy(&m_event_grp, &m_data[PAYLOAD_POS + 14], sizeof(m_event_grp));
            for (uint32_t i = 1; (i * 2 + PAYLOAD_POS + 16) <= getMessageLen(); ++i) {
                event_t eid;
                memcpy(&eid, &m_data[(i - 1) * 2 + PAYLOAD_POS + 16], sizeof(event_t));
                m_events.insert(eid);
            }
        }

        std::string getCmdString() {
            std::string str = 
                "SOMEIP_REQUEST_EVENT_ACK ["
                + std::to_string(m_timestamp)
                + "]("
                + std::to_string(m_srv_id)
                + "," + std::to_string(m_ins_id)
                + "," + std::to_string(m_major)
                + ") group: " + std::to_string(m_event_grp) + "[";
            int i = 0;
            for (auto et : m_events) {
                if (i == 0) {
                    str += std::to_string(et);
                }
                else {
                    str += "," + std::to_string(et);
                }
                ++i;
            }
            str += "] sub group avail_state = " + 
                std::to_string(static_cast<int>(m_is_avail));
            return str;
        };
        uint64_t getTimeStamp() { return m_timestamp; }
        uint16_t getSrvid() { return m_srv_id; }
        uint16_t getInsid() { return m_ins_id; }
        uint8_t getMajor() { return m_major; }
        e_evt_ack_type_t getIsAvail() { return m_is_avail; }
        eventgroup_t getEvtGrp() { return m_event_grp; }
        std::set<event_t>& getEvts() { return m_events; }

    private:
        uint64_t             m_timestamp;
        uint16_t             m_srv_id;
        uint16_t             m_ins_id;
        uint8_t              m_major;
        e_evt_ack_type_t     m_is_avail{ e_evt_ack_type_t::E_EXCEPTION };
        eventgroup_t         m_event_grp;
        std::set<event_t>    m_events;
    };

    /* Start [GAC SPEC] upload Major version / interface version error to APP*/
    /* SOMEIP_EXP_MAJOR_VER */
    class ExpMajorVersionCmd : public AppMessage {
    public:
        enum err_type :uint8_t {
            unknown        = 0,
            offer_err      = 1,     // RX offer error one time
            sub_err        = 2,     // RX sub error one time
            request_err    = 3,     // RX request message error one time
            responnse_err  = 4,     // RX response message error one time
            notify_err     = 5,     // RX notify message error one time
            offer_exp      = 6,     // RX offer error 3 times exception
            sub_exp        = 7,     // RX sub error 3 times exception
            request_exp    = 8,     // RX request message error 3 times exception
            responnse_exp  = 9,     // RX response message error 3 times exception
            notify_exp     = 10,    // RX notify message error 3 times exception
        };
        ExpMajorVersionCmd(uint16_t _srv_id, uint16_t _ins_id, uint8_t _maj_ver, uint8_t _err_maj_ver,
            err_type _type, uint16_t _port, std::string _rmt_ip, uint16_t _method_id = 0) :
            AppMessage(APP_HEADER_LEN),
            m_srv_id(_srv_id),
            m_ins_id(_ins_id),
            m_major(_maj_ver),
            m_err_major(_err_maj_ver),
            m_err_type(_type),
            m_rmt_port(_port),
            m_rmt_ip(_rmt_ip),
            m_method_id(_method_id) {
            setMessageLen(APP_HEADER_LEN + 19 + _rmt_ip.length());
            setCommandId(SOMEIP_EXP_MAJOR_VER);
            auto now = std::chrono::system_clock::now();
            m_timestamp = std::chrono::duration_cast<
                std::chrono::milliseconds>(now.time_since_epoch()).count();
            memcpy(&m_data[PAYLOAD_POS], &m_timestamp, sizeof(m_timestamp));
            memcpy(&m_data[PAYLOAD_POS + 8], &_srv_id, sizeof(_srv_id));
            memcpy(&m_data[PAYLOAD_POS + 10], &_ins_id, sizeof(_ins_id));
            memcpy(&m_data[PAYLOAD_POS + 12], &_maj_ver, sizeof(_maj_ver));
            memcpy(&m_data[PAYLOAD_POS + 13], &_err_maj_ver, sizeof(_err_maj_ver));
            memcpy(&m_data[PAYLOAD_POS + 14], &_type, sizeof(_type));
            memcpy(&m_data[PAYLOAD_POS + 15], &_method_id, sizeof(_method_id));
            memcpy(&m_data[PAYLOAD_POS + 17], &_port, sizeof(_port));
            memcpy(&m_data[PAYLOAD_POS + 19], _rmt_ip.c_str(), _rmt_ip.length());
        }

        ExpMajorVersionCmd(const size_t _in_len) :AppMessage(_in_len) {
            setCommandId(SOMEIP_EXP_MAJOR_VER);
        }

        ~ExpMajorVersionCmd() {}
        void unserializePayload() {
            if (m_data.size() < APP_HEADER_LEN + 19) {
                return;
            }
            memcpy(&m_timestamp, &m_data[PAYLOAD_POS], sizeof(m_timestamp));
            memcpy(&m_srv_id, &m_data[PAYLOAD_POS + 8], sizeof(m_srv_id));
            memcpy(&m_ins_id, &m_data[PAYLOAD_POS + 10], sizeof(m_ins_id));
            memcpy(&m_major, &m_data[PAYLOAD_POS + 12], sizeof(m_major));
            memcpy(&m_err_major, &m_data[PAYLOAD_POS + 13], sizeof(m_err_major));
            memcpy(&m_err_type, &m_data[PAYLOAD_POS + 14], sizeof(m_err_type));
            memcpy(&m_method_id, &m_data[PAYLOAD_POS + 15], sizeof(m_method_id));
            memcpy(&m_rmt_port, &m_data[PAYLOAD_POS + 17], sizeof(m_rmt_port));
            m_rmt_ip.resize(m_data.size() - APP_HEADER_LEN - 19 + 1, 0);
            for (size_t i = 0; i < m_data.size() - APP_HEADER_LEN - 19; ++i) {
                m_rmt_ip[i] = m_data[PAYLOAD_POS + 19 + i];
            }

        }

        std::string getCmdString() {
            static std::map<err_type, std::string> err_name = {
                {err_type::unknown, "unknown" },
                {err_type::offer_err, "offer_err" },
                {err_type::sub_err, "sub_err" },
                {err_type::request_err, "request_err" },
                {err_type::responnse_err, "responnse_err" },
                {err_type::notify_err, "responnse_err" },
                {err_type::offer_exp, "offer_exp" },
                {err_type::sub_exp, "sub_exp" },
                {err_type::request_exp, "request_exp" },
                {err_type::responnse_exp, "responnse_exp" },
                {err_type::notify_exp, "notify_exp" },
            };
            std::string str =
                "SOMEIP_EXP_MAJOR_VER ["
                + std::to_string(m_timestamp) + "]"
                + err_name[m_err_type]
                + "(" + std::to_string(m_srv_id)
                + "," + std::to_string(m_ins_id)
                + "," + std::to_string(m_major)
                + "/" + std::to_string(m_err_major)
                + ",method/event:"+ std::to_string(m_method_id) + ") from remote " + m_rmt_ip.c_str()
                + ":" + std::to_string(m_rmt_port);
            return str;
        };
        uint64_t getTimeStamp() { return m_timestamp; }
        uint16_t getSrvid() { return m_srv_id; }
        uint16_t getInsid() { return m_ins_id; }
        uint8_t getMajor() { return m_major; }
        uint8_t getErrMajor() { return m_err_major; }
        err_type getErrType() { return m_err_type; }
        uint16_t getRmtPort() { return m_rmt_port; }
        std::string getRmtIp() { return m_rmt_ip; }
        uint16_t getMethodId() { return m_method_id; }

    private:
        uint64_t        m_timestamp;
        uint16_t        m_srv_id;
        uint16_t        m_ins_id;
        uint8_t         m_major;
        uint8_t         m_err_major;
        err_type        m_err_type{ err_type::unknown };
        uint16_t        m_rmt_port;
        std::string     m_rmt_ip;
        uint16_t        m_method_id;
    };
    /* End [GAC SPEC] upload Major version / interface version error to APP*/

    /* payload struct for below command
        SOMEIP_REGISTER_EVENT
        SOMEIP_OFFER_EVENT
        SOMEIP_UNREGISTER_EVENT
        SOMEIP_STOP_OFFER_EVENT
        SOMEIP_OFFERED_EVENT_ACK */
    class AppMsgEventCmd : public AppMessage {
        const uint16_t LEN = sizeof(eventgroup_t);
    public:
        AppMsgEventCmd(uint8_t _cmd_id, uint16_t _srv_id, uint16_t _ins_id, uint8_t _maj_ver = 1,
            uint16_t _event = 0, std::set<eventgroup_t> _eventGroups = std::set<eventgroup_t>{},
            uint8_t _isProvided = 0, uint8_t _type = 0, uint8_t _reliability = 0) :
            AppMessage(APP_EVENT_MSG_LEN),
            m_srv_id(_srv_id),
            m_ins_id(_ins_id),
            m_major(_maj_ver),
            m_event(_event),
            m_isProvided(_isProvided),
            m_eventGroups(_eventGroups),
            m_type(_type),
            m_reliability(_reliability) {
            size_t size = _eventGroups.size();
            setMessageLen(APP_EVENT_MSG_LEN + size * LEN);
            setCommandId(_cmd_id);
            memcpy(&m_data[PAYLOAD_POS], &_srv_id, sizeof(_srv_id));
            memcpy(&m_data[PAYLOAD_POS + 2], &_ins_id, sizeof(_ins_id));
            memcpy(&m_data[PAYLOAD_POS + 4], &_maj_ver, sizeof(_maj_ver));
            memcpy(&m_data[PAYLOAD_POS + 5], &_event, sizeof(_event));
            memcpy(&m_data[PAYLOAD_POS + 7], &_isProvided, sizeof(_isProvided));
            memcpy(&m_data[PAYLOAD_POS + 8], &_type, sizeof(_type));
            memcpy(&m_data[PAYLOAD_POS + 9], &_reliability, sizeof(_reliability));
            uint16_t i = 0;
            for(auto group : _eventGroups) {
                memcpy(&m_data[PAYLOAD_POS + 10 + i*LEN], &group, sizeof(eventgroup_t));
                ++i;
            }
        }

        AppMsgEventCmd(uint8_t _cmd_id, const size_t _in_len) :AppMessage(_in_len) {
            setCommandId(_cmd_id);
        }

        ~AppMsgEventCmd() {}
        void unserializePayload() {
            if (m_data.size() < APP_EVENT_MSG_LEN) {
                return; 
            }
            memcpy(&m_srv_id, &m_data[PAYLOAD_POS], sizeof(m_srv_id));
            memcpy(&m_ins_id, &m_data[PAYLOAD_POS + 2], sizeof(m_ins_id));
            memcpy(&m_major, &m_data[PAYLOAD_POS + 4], sizeof(m_major));
            memcpy(&m_event, &m_data[PAYLOAD_POS + 5], sizeof(m_event));
            memcpy(&m_isProvided, &m_data[PAYLOAD_POS + 7], sizeof(m_isProvided));
            memcpy(&m_type, &m_data[PAYLOAD_POS + 8], sizeof(m_type));
            memcpy(&m_reliability, &m_data[PAYLOAD_POS + 9], sizeof(m_reliability));

            for (uint32_t i = 1; (i * 2 + PAYLOAD_POS + 10) <= getMessageLen(); ++i) {
                eventgroup_t grp;
                memcpy(&grp, &m_data[(i-1) * 2 + PAYLOAD_POS + 10], sizeof(eventgroup_t));
                m_eventGroups.insert(grp);
            }
        }

        void setSrvid(uint16_t _id) {
            m_srv_id = _id;
            memcpy(&m_data[PAYLOAD_POS], &m_srv_id, sizeof(m_srv_id));
        }
        uint16_t getSrvid() { return m_srv_id; }

        void setInsid(uint16_t _id) {
            m_ins_id = _id;
            memcpy(&m_data[PAYLOAD_POS + 2], &m_ins_id, sizeof(m_ins_id));
        }
        uint16_t getInsid() { return m_ins_id; }

        void setMajor(uint8_t _id) {
            m_major = _id;
            memcpy(&m_data[PAYLOAD_POS + 4], &m_major, sizeof(m_major));
        }
        uint8_t getMajor() { return m_major; }

        void setEvent(uint16_t _id) {
            m_event = _id;
            memcpy(&m_data[PAYLOAD_POS + 5], &m_event, sizeof(m_event));
        }
        uint16_t getEvent() { return m_event; }

        void setEventGroup(std::set<eventgroup_t> _eventGroups) {
            m_eventGroups = _eventGroups;
            uint16_t i = 0;
            for(auto group : m_eventGroups) {
                memcpy(&m_data[i * 2 + PAYLOAD_POS + 10], &group, sizeof(group));
                ++i;
            }
        }
        std::set<eventgroup_t> getEventGroup() { return m_eventGroups; }

        void setIsProvided(uint8_t _isProvided) {
            m_isProvided = _isProvided;
            memcpy(&m_data[PAYLOAD_POS + 7], &m_isProvided, sizeof(m_isProvided));
        }
        uint8_t getIsProvided() { return m_isProvided; }

        void setType(uint8_t _type) {
            m_type = _type;
            memcpy(&m_data[PAYLOAD_POS + 8], &m_type, sizeof(m_type));
        }
        uint8_t getType() { return m_type; }

        void setReliability(uint8_t _reliability) {
            m_reliability = _reliability;
            memcpy(&m_data[PAYLOAD_POS + 9], &m_reliability, sizeof(m_reliability));
        }
        uint8_t getReliability() { return m_reliability; }
        std::string getCmdString() {
            std::map<uint8_t, std::string> cmd_name = {
                {SOMEIP_REGISTER_EVENT, "SOMEIP_REGISTER_EVENT"},
                {SOMEIP_OFFER_EVENT, "SOMEIP_OFFER_EVENT"},
                {SOMEIP_UNREGISTER_EVENT, "SOMEIP_UNREGISTER_EVENT"},
                {SOMEIP_STOP_OFFER_EVENT, "SOMEIP_STOP_OFFER_EVENT"},
                {SOMEIP_OFFERED_EVENT_ACK, "SOMEIP_OFFERED_EVENT_ACK"},
            };

            std::string str_grp = "[";
            for (auto group : m_eventGroups) {
                str_grp += std::to_string(group) + " ";
            }
            str_grp += "]";
            std::string str = cmd_name[getCommandId()] + "("
                + std::to_string(m_srv_id) + ","
                + std::to_string(m_ins_id) + ","
                + std::to_string(m_major) + ") event/group:("
                + std::to_string(m_event) + str_grp
                + ")";
            return str;
        };
    private:
        uint16_t    m_srv_id;
        uint16_t    m_ins_id;
        uint8_t     m_major;
        uint16_t    m_event;
        uint8_t     m_isProvided;
        std::set<eventgroup_t>    m_eventGroups;
        uint8_t     m_type;
        uint8_t     m_reliability;
    };

    /* payload struct for below command
        SOMEIP_AVAILABLE_SERVICE */
    class AppAvailableMsg : public AppMessage {
    public:
        AppAvailableMsg(service_t _srv_id, instance_t _ins_id, major_t _major, minor_t _minor, 
            bool _is_avail) :
            AppMessage(APP_SERVICE_AVAILABLE_LEN),
            m_srv_id(_srv_id),
            m_ins_id(_ins_id),
            m_major(_major),
            m_minor(_minor),
            m_is_avail(_is_avail){
            setCommandId(SOMEIP_AVAILABLE_SERVICE);
            memcpy(&m_data[PAYLOAD_POS], &_srv_id, sizeof(_srv_id));
            memcpy(&m_data[PAYLOAD_POS + 2], &_ins_id, sizeof(_ins_id));
            memcpy(&m_data[PAYLOAD_POS + 4], &_major, sizeof(_major));
            memcpy(&m_data[PAYLOAD_POS + 5], &_minor, sizeof(_minor));
            memcpy(&m_data[PAYLOAD_POS + 9], &_is_avail, sizeof(_is_avail));
        }
        AppAvailableMsg() :AppMessage(APP_SERVICE_AVAILABLE_LEN) {
            setCommandId(SOMEIP_AVAILABLE_SERVICE);
        }

        ~AppAvailableMsg() {}
        void unserializePayload() {
            memcpy(&m_srv_id, &m_data[PAYLOAD_POS], sizeof(m_srv_id));
            memcpy(&m_ins_id, &m_data[PAYLOAD_POS + 2], sizeof(m_ins_id));
            memcpy(&m_major, &m_data[PAYLOAD_POS + 4], sizeof(m_major));
            memcpy(&m_minor, &m_data[PAYLOAD_POS + 5], sizeof(m_minor));
            memcpy(&m_is_avail, &m_data[PAYLOAD_POS + 9], sizeof(m_is_avail));
        }

        void setSrvid(service_t _id) {
            m_srv_id = _id;
            memcpy(&m_data[PAYLOAD_POS], &m_srv_id, sizeof(m_srv_id));
        }
        service_t getSrvid() { return m_srv_id; }

        void setInsid(instance_t _id) {
            m_ins_id = _id;
            memcpy(&m_data[PAYLOAD_POS + 2], &m_ins_id, sizeof(m_ins_id));
        }
        instance_t getInsid() { return m_ins_id; }

        void setMajor(major_t _id) {
            m_major = _id;
            memcpy(&m_data[PAYLOAD_POS + 4], &m_major, sizeof(m_major));
        }
        major_t getMajor() { return m_major; }

        void setMinor(minor_t _id) {
            m_minor = _id;
            memcpy(&m_data[PAYLOAD_POS + 5], &m_minor, sizeof(m_minor));
        }
        minor_t getMinor() { return m_minor; }

        void setIsAvail(bool _is_avail) {
            m_is_avail = _is_avail;
            memcpy(&m_data[PAYLOAD_POS + 9], &m_is_avail, sizeof(m_is_avail));
        }
        bool getIsAvail() { return m_is_avail; }
		std::string getCmdString() { 
            std::string str = "SOMEIP_AVAILABLE_SERVICE("
                + std::to_string(m_srv_id) + ","
                + std::to_string(m_ins_id) + ","
                + std::to_string(m_major) + ","
                + std::to_string(m_minor) + "): "
                + std::to_string(m_is_avail);
            return str; 
        };

    private:
        service_t    m_srv_id;
        instance_t   m_ins_id;
        major_t      m_major;
        minor_t      m_minor;
        bool         m_is_avail;
    };

    /* payload struct for below command
        SOMEIP_SUBSCRIBE
        SOMEIP_UNSUBSCRIBE */
    class AppMsgSubscribeCmd : public AppMessage {
    public:
        AppMsgSubscribeCmd(uint8_t _cmd_id, uint16_t _srv_id, uint16_t _ins_id, 
            uint16_t _event, uint16_t _eventGroup, uint16_t _pending = 0, uint8_t _major = 0) :
            AppMessage(APP_SUBSCRIBE_MSG_LEN),
            m_srv_id(_srv_id),
            m_ins_id(_ins_id),
            m_event(_event),
            m_eventGroup(_eventGroup),
            m_pending(_pending),            
            m_major(_major) {
            setCommandId(_cmd_id);
            memcpy(&m_data[PAYLOAD_POS], &_srv_id, sizeof(_srv_id));
            memcpy(&m_data[PAYLOAD_POS + 2], &_ins_id, sizeof(_ins_id));
            memcpy(&m_data[PAYLOAD_POS + 4], &_event, sizeof(_event));
            memcpy(&m_data[PAYLOAD_POS + 6], &_eventGroup, sizeof(_eventGroup));
            memcpy(&m_data[PAYLOAD_POS + 8], &_pending, sizeof(_pending));
            memcpy(&m_data[PAYLOAD_POS + 10], &_major, sizeof(_major));
        }
        AppMsgSubscribeCmd(uint8_t _cmd_id) :AppMessage(APP_SUBSCRIBE_MSG_LEN) {
            setCommandId(_cmd_id);
        }

        ~AppMsgSubscribeCmd() {}
        void unserializePayload() {
            memcpy(&m_srv_id, &m_data[PAYLOAD_POS], sizeof(m_srv_id));
            memcpy(&m_ins_id, &m_data[PAYLOAD_POS + 2], sizeof(m_ins_id));
            memcpy(&m_event, &m_data[PAYLOAD_POS + 4], sizeof(m_event));
            memcpy(&m_eventGroup, &m_data[PAYLOAD_POS + 6], sizeof(m_eventGroup));
            memcpy(&m_pending, &m_data[PAYLOAD_POS + 8], sizeof(m_pending));
            memcpy(&m_major, &m_data[PAYLOAD_POS + 10], sizeof(m_major));
        }

        void setSrvid(uint16_t _id) {
            m_srv_id = _id;
            memcpy(&m_data[PAYLOAD_POS], &m_srv_id, sizeof(m_srv_id));
        }
        uint16_t getSrvid() { return m_srv_id; }

        void setInsid(uint16_t _id) {
            m_ins_id = _id;
            memcpy(&m_data[PAYLOAD_POS + 2], &m_ins_id, sizeof(m_ins_id));
        }
        uint16_t getInsid() { return m_ins_id; }

        void setEvent(uint16_t _id) {
            m_event = _id;
            memcpy(&m_data[PAYLOAD_POS + 4], &m_event, sizeof(m_event));
        }
        uint16_t getEvent() { return m_event; }

        void setEventGroup(uint16_t _id) {
            m_eventGroup = _id;
            memcpy(&m_data[PAYLOAD_POS + 6], &m_eventGroup, sizeof(m_eventGroup));
        }
        uint16_t getEventGroup() { return m_eventGroup; }

        void setPending(uint16_t _id) {
            m_pending = _id;
            memcpy(&m_data[PAYLOAD_POS + 8], &m_pending, sizeof(m_pending));
        }
        uint16_t getPending() { return m_pending; }

        void setMajor(uint8_t _id) {
            m_major = _id;
            memcpy(&m_data[PAYLOAD_POS + 10], &m_major, sizeof(m_major));
        }
        uint8_t getMajor() { return m_major; }
        std::string getCmdString() {
            std::map<uint8_t, std::string> cmd_name = {
                {SOMEIP_SUBSCRIBE, "SOMEIP_SUBSCRIBE"},
                {SOMEIP_UNSUBSCRIBE, "SOMEIP_UNSUBSCRIBE"},
            };
            std::string str = cmd_name[getCommandId()] + "("
                + std::to_string(m_srv_id) + ","
                + std::to_string(m_ins_id) + ","
                + std::to_string(m_major) + ") event/group:("
                + std::to_string(m_event) + std::to_string(m_eventGroup)
                + ")";
            return str;
        };
    private:
        uint16_t    m_srv_id;
        uint16_t    m_ins_id;
        uint16_t    m_event;
        uint16_t    m_eventGroup;
        uint16_t    m_pending;
        uint8_t     m_major;
    };

    /* payload struct for below command
         SOMEIP_SEND */
    class AppMsgSomeipSendCmd : public AppMessage {
    public:
        const uint32_t SEND_CMD_HEADER_LEN = 5; // srvid(2) + insid(2) + e2e stat(1)
        AppMsgSomeipSendCmd(size_t _len) :AppMessage(_len) {
            if (_len > APP_HEADER_LEN) {
                setCommandId(SOMEIP_SEND);
                m_msg = std::make_shared< apf::someip::SomeIpMessage<> >();
            }
        }
        AppMsgSomeipSendCmd(uint16_t _id1, uint16_t _id2,
            std::shared_ptr<apf::someip::SomeIpMessage<>> _msg, 
            e2eCheckStat _e2eRt = e2eCheckStat::E2E_STATUS_OK) :AppMessage(0) {
            m_msg = _msg;
            //m_serializer << *m_msg;
            //m_serializer << m_msg->getMessageHeader();
            size_t len = 16 + m_msg->getMessagePayload().size();
            // head + srvid + insid + someip message
            setMessageLen(len + APP_HEADER_LEN + SEND_CMD_HEADER_LEN); // clear data
            setCommandId(SOMEIP_SEND);
            setPayloadLen(static_cast<uint32_t>(len + SEND_CMD_HEADER_LEN));
            m_srv_id = _id1;
            m_ins_id = _id2;
            m_e2e_stat = _e2eRt;
            m_major_ver = m_msg->getMessageHeader().getInterfaceVersion();
            memcpy(&m_data[PAYLOAD_POS], &m_srv_id, sizeof(m_srv_id));
            memcpy(&m_data[PAYLOAD_POS + 2], &m_ins_id, sizeof(m_ins_id));
            memcpy(&m_data[PAYLOAD_POS + 4], &m_e2e_stat, sizeof(m_e2e_stat));
            // set someip message header

            uint32_t msgId = m_msg->getMessageHeader().getMessageId();
            uint32_t smip_len = m_msg->getMessageHeader().getLength();
            uint32_t smip_req_id = m_msg->getMessageHeader().getRequestId();
            uint8_t smip_proto_ver = m_msg->getMessageHeader().getProtocolVersion();
            uint8_t smip_if_ver = m_msg->getMessageHeader().getInterfaceVersion();
            uint8_t smip_msg_type = static_cast<uint8_t>(m_msg->getMessageHeader().getMessageType());
            uint8_t smip_ret_code = static_cast<uint8_t>(m_msg->getMessageHeader().getReturnCode());
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN], &msgId, sizeof(msgId));
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 4], &smip_len, sizeof(smip_len));
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 8], &smip_req_id, sizeof(smip_req_id));
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 12], &smip_proto_ver, sizeof(smip_proto_ver));
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 13], &smip_if_ver, sizeof(smip_if_ver));
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 14], &smip_msg_type, sizeof(smip_msg_type));
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 15], &smip_ret_code, sizeof(smip_ret_code));

            /*std::copy(m_serializer.m_buffer.begin(), m_serializer.m_buffer.begin()+ m_serializer.getSize(),
                m_data.begin() + APP_HEADER_LEN + SEND_CMD_HEADER_LEN);*/
            // set payload
            if (m_msg->getMessagePayload().size() > 0) {
                memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 16],
                    &m_msg->getMessagePayload()[0], m_msg->getMessagePayload().size());
            }
        }
        void unserializePayload() {
            memcpy(&m_srv_id, &m_data[PAYLOAD_POS], sizeof(m_srv_id));
            memcpy(&m_ins_id, &m_data[PAYLOAD_POS + 2], sizeof(m_ins_id));
            uint8_t tmp_e2e_ret = 0;
            memcpy(&tmp_e2e_ret, &m_data[PAYLOAD_POS + 4], sizeof(tmp_e2e_ret));
            m_e2e_stat = apf::utility::safeIntCast<e2eCheckStat>(tmp_e2e_ret);
            if (m_data.size() >= (APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 16)) {
                /*apf::Deserializer deserializer(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN]
                    , static_cast<uint32_t>(m_data.size() - APP_HEADER_LEN - SEND_CMD_HEADER_LEN));
                deserializer >> *m_msg;*/
                uint32_t msgId;
                uint32_t smip_len;
                uint32_t smip_req_id;
                uint8_t smip_proto_ver;
                uint8_t smip_if_ver;
                uint8_t smip_msg_type;
                uint8_t smip_ret_code;
                memcpy(&msgId, &m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN], sizeof(msgId));
                memcpy(&smip_len, &m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 4], sizeof(smip_len));
                memcpy(&smip_req_id, &m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 8], sizeof(smip_req_id));
                memcpy(&smip_proto_ver, &m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 12], sizeof(smip_proto_ver));
                memcpy(&smip_if_ver, &m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 13], sizeof(smip_if_ver));
                memcpy(&smip_msg_type, &m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 14], sizeof(smip_msg_type));
                memcpy(&smip_ret_code, &m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 15], sizeof(smip_ret_code));

                apf::someip::MessageStandardHeader msg_header;
                msg_header.setMessageId(msgId);
                msg_header.setLength(smip_len);
                msg_header.setRequestId(smip_req_id);
                msg_header.setProtocolVersion(smip_proto_ver);
                msg_header.setInterfaceVersion(smip_if_ver);
                msg_header.setMessageType(static_cast<apf::someip::MessageStandardHeader::MessageType>(smip_msg_type));
                msg_header.setReturnCode(static_cast<apf::someip::MessageStandardHeader::ReturnCode>(smip_ret_code));

                m_msg->setMessageHeader(msg_header);
                std::vector<uint8_t> payload(m_data.begin() + APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 16,
                    m_data.end());
                m_msg->setMessagePayload(payload);

                m_major_ver = smip_if_ver;
            }
        }

        void setSomeipMsg(std::shared_ptr<apf::someip::SomeIpMessage<>> _msg) {
            m_msg = _msg;
            //m_serializer << *m_msg;
            //m_serializer << m_msg->getMessageHeader();
            size_t len = 16 + m_msg->getMessagePayload().size();
            // head + srvid + insid + someip message
            setMessageLen(len + APP_HEADER_LEN + SEND_CMD_HEADER_LEN); // clear data
            setCommandId(SOMEIP_SEND);
            setPayloadLen(static_cast<uint32_t>(len + SEND_CMD_HEADER_LEN));
            memcpy(&m_data[PAYLOAD_POS], &m_srv_id, sizeof(m_srv_id));
            memcpy(&m_data[PAYLOAD_POS + 2], &m_ins_id, sizeof(m_ins_id));
            // set someip message data into m_data
            //m_serializer.getBuffer(&m_data[APP_HEADER_LEN + KSOMEIP_SRV_INS_LEN], len);
            // set someip message header
            /*std::copy(m_serializer.m_buffer.begin(), m_serializer.m_buffer.begin() + m_serializer.getSize(),
                m_data.begin() + APP_HEADER_LEN + SEND_CMD_HEADER_LEN);*/

            uint32_t msgId = m_msg->getMessageHeader().getMessageId();
            uint32_t smip_len = m_msg->getMessageHeader().getLength();
            uint32_t smip_req_id = m_msg->getMessageHeader().getRequestId();
            uint8_t smip_proto_ver = m_msg->getMessageHeader().getProtocolVersion();
            uint8_t smip_if_ver = m_msg->getMessageHeader().getInterfaceVersion();
            uint8_t smip_msg_type = static_cast<uint8_t>(m_msg->getMessageHeader().getMessageType());
            uint8_t smip_ret_code = static_cast<uint8_t>(m_msg->getMessageHeader().getReturnCode());
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN], &msgId, sizeof(msgId));
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 4], &smip_len, sizeof(smip_len));
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 8], &smip_req_id, sizeof(smip_req_id));
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 12], &smip_proto_ver, sizeof(smip_proto_ver));
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 13], &smip_if_ver, sizeof(smip_if_ver));
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 14], &smip_msg_type, sizeof(smip_msg_type));
            memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 15], &smip_ret_code, sizeof(smip_ret_code));
            m_major_ver = smip_if_ver;
            // set payload
            if (m_msg->getMessagePayload().size() > 0) {
                memcpy(&m_data[APP_HEADER_LEN + SEND_CMD_HEADER_LEN + 16],
                    &m_msg->getMessagePayload()[0], m_msg->getMessagePayload().size());
            }
        }

        ~AppMsgSomeipSendCmd() {}

        uint16_t getSrvid() { return m_srv_id; }
        uint16_t getInsid() { return m_ins_id; }
        uint8_t getMajor() { return m_major_ver; }
        e2eCheckStat getE2eStat() { return m_e2e_stat; }

        std::shared_ptr<apf::someip::SomeIpMessage<>> getSomeipMessage() {
            return m_msg;
        }
        std::string getCmdString() {
            std::string str = "SOMEIP_SEND("
                + std::to_string(m_srv_id) + ","
                + std::to_string(m_ins_id) + ","
                + std::to_string(m_major_ver) + "):" 
                + std::to_string(m_msg->getMessageHeader().getMethodEventId())
                + std::to_string(static_cast<int>(m_msg->getMessageHeader().getMessageType()));
            return str;
        };

    private:
        std::shared_ptr<apf::someip::SomeIpMessage<>> m_msg;
        //apf::Serializer  m_serializer;
        uint16_t         m_srv_id;
        uint16_t         m_ins_id;
        e2eCheckStat     m_e2e_stat;
        uint8_t          m_major_ver{1};
    };

    /*Start [GAC SPEC] request get service status */
    /* payload struct for below command
        SOMEIP_SERVICE_STATUS_REQ */
    class AppMsgRequestGetServiceStatus : public AppMessage {
    public:
        enum class REQ_TYPE :uint8_t {
            ALL_SERVER  = 0,
            ALL_CLIENT  = 1,
            SIG_SERVICE = 2,
            ALL_SERVICE = 3,
            ERROR_TYPE
        };
        AppMsgRequestGetServiceStatus(REQ_TYPE _type) :
            AppMessage(APP_HEADER_LEN+1),
            m_req_type(_type){
            setCommandId(SOMEIP_SERVICE_STATUS_REQ);
            memcpy(&m_data[PAYLOAD_POS], &m_req_type, sizeof(m_req_type));
        }
        AppMsgRequestGetServiceStatus() :AppMessage(APP_HEADER_LEN+1) {
            setCommandId(SOMEIP_SERVICE_STATUS_REQ);
        }

        ~AppMsgRequestGetServiceStatus() {}
        void unserializePayload() {
            uint8_t type;
            memcpy(&type, &m_data[PAYLOAD_POS], sizeof(type));
            m_req_type = static_cast<REQ_TYPE>(type);
        }
        uint16_t getSrvid() { return 0; }
        uint16_t getInsid() { return 0; }
        uint8_t getMajor() { return 0; }
        std::string getCmdString() {
            std::map<REQ_TYPE, std::string> cmd_type = {
                {REQ_TYPE::ALL_SERVER, "ALL_SERVER"},
                {REQ_TYPE::ALL_CLIENT, "ALL_CLIENT"},
                {REQ_TYPE::SIG_SERVICE, "SIG_SERVICE"},
                {REQ_TYPE::ALL_SERVICE, "ALL_SERVICE"},
            };
            std::string str = "SOMEIP_SERVICE_STATUS_REQ " + cmd_type[m_req_type];
            return str;
        };
        REQ_TYPE getReqType() {
            return m_req_type;
        }

    private:
        REQ_TYPE   m_req_type;
    };
    class AppMsgGetServiceStatusRsp : public AppMessage {
        const uint16_t LEN = 6; // service id + instance id + major ver + is_avail
    public:
        AppMsgGetServiceStatusRsp(AppMsgRequestGetServiceStatus::REQ_TYPE _req_type,
            std::map<serviceInstanceId, bool>& _service_status) :
            AppMessage(APP_HEADER_LEN),
            m_req_type(_req_type),
            m_service_status(_service_status){
            setMessageLen(APP_HEADER_LEN + 1 + m_service_status.size() * LEN);
            setCommandId(SOMEIP_SERVICE_STATUS_RSP);
            memcpy(&m_data[PAYLOAD_POS], &m_req_type, sizeof(m_req_type));
            uint16_t i = 0;
            for (auto srv_stat : _service_status) {
                memcpy(&m_data[PAYLOAD_POS + 1 + i * LEN], &srv_stat.first.id1, sizeof(srv_stat.first.id1));
                memcpy(&m_data[PAYLOAD_POS + 1 + i * LEN + 2], &srv_stat.first.id2, sizeof(srv_stat.first.id2));
                memcpy(&m_data[PAYLOAD_POS + 1 + i * LEN + 4], &srv_stat.first.id3, sizeof(srv_stat.first.id3));
                memcpy(&m_data[PAYLOAD_POS + 1 + i * LEN + 5], &srv_stat.second, sizeof(bool));
                ++i;
            }
        }
        AppMsgGetServiceStatusRsp(const size_t _in_len) :AppMessage(_in_len) {
            setCommandId(SOMEIP_SERVICE_STATUS_RSP);
        }

        ~AppMsgGetServiceStatusRsp() {}
        void unserializePayload() {
            memcpy(&m_req_type, &m_data[PAYLOAD_POS], sizeof(m_req_type));
            auto map_len = m_data.size() - 1 - APP_HEADER_LEN;
            if ((map_len % LEN) != 0) {
                return;
            }
            auto map_size = map_len / LEN;
            for (size_t i = 0; i < map_size; ++i) {
                serviceInstanceId srv;
                bool srv_stat = false;
                memcpy(&srv.id1, &m_data[PAYLOAD_POS + 1 + i * LEN], sizeof(srv.id1));
                memcpy(&srv.id2, &m_data[PAYLOAD_POS + 1 + i * LEN + 2], sizeof(srv.id2));
                memcpy(&srv.id3, &m_data[PAYLOAD_POS + 1 + i * LEN + 4], sizeof(srv.id3));
                memcpy(&srv_stat, &m_data[PAYLOAD_POS + 1 + i * LEN + 5], sizeof(srv_stat));
                m_service_status[srv] = srv_stat;
            }
        }
        uint16_t getSrvid() { return 0; }
        uint16_t getInsid() { return 0; }
        uint8_t getMajor() { return 0; }
        std::string getCmdString() {
            std::map<AppMsgRequestGetServiceStatus::REQ_TYPE, std::string> cmd_type = {
                {AppMsgRequestGetServiceStatus::REQ_TYPE::ALL_SERVER, "ALL_SERVER"},
                {AppMsgRequestGetServiceStatus::REQ_TYPE::ALL_CLIENT, "ALL_CLIENT"},
                {AppMsgRequestGetServiceStatus::REQ_TYPE::SIG_SERVICE, "SIG_SERVICE"},
                {AppMsgRequestGetServiceStatus::REQ_TYPE::ALL_SERVICE, "ALL_SERVICE"},
            };
            std::string srv_str;
            for (auto srv : m_service_status) {
                std::string stat = (srv.second) ? "true":"false" ;
                srv_str += "(" + std::to_string(srv.first.id1)
                    + "," + std::to_string(srv.first.id2)
                    + "," + std::to_string(srv.first.id3)
                    + ":" + stat + ")";
            }
            std::string str = "[SOMEIP_SERVICE_STATUS_RSP] Requset type :"
                + cmd_type[m_req_type] + " service_status:" + srv_str;
            return str;
        };
        std::map<serviceInstanceId, bool>& getServiceStatus() {
            return m_service_status;
        }
        AppMsgRequestGetServiceStatus::REQ_TYPE getReqType() {
            return m_req_type;
        }
    private:
        AppMsgRequestGetServiceStatus::REQ_TYPE m_req_type;
        std::map<serviceInstanceId, bool> m_service_status;
    };
    /*End [GAC SPEC] service status response*/
    class AppSecialMsgMgr {
    public:
        static bool allocSpecifiedMsgByCmdId(uint8_t _cmd_id, const size_t _in_len,
            std::shared_ptr<AppMessage>& _msg) {
            bool ret = true;
            switch (_cmd_id) {
            case SOMEIP_REQUEST_APPID:
            case SOMEIP_DEREGISTER_APPLICATION:
            {
                _msg = std::make_shared<AppMsgAppId>(_cmd_id);
                break;
            }
            case SOMEIP_REQUEST_APPID_ACK:
            {
                _msg = std::make_shared<AppMsgRequestAppIdAckData>();
                break;
            }
            case SOMEIP_OFFER_SERVICE:
            case SOMEIP_STOP_OFFER_SERVICE:
            case SOMEIP_REQUEST_SERVICE:
            case SOMEIP_RELEASE_SERVICE:
            {
                _msg = std::make_shared<AppMsgServiceCmd>(_cmd_id);
                break;
            }
            case SOMEIP_REGISTER_EVENT:
            case SOMEIP_UNREGISTER_EVENT:
            case SOMEIP_OFFER_EVENT:
            case SOMEIP_STOP_OFFER_EVENT:
            case SOMEIP_OFFERED_EVENT_ACK:
            {
                _msg = std::make_shared<AppMsgEventCmd>(_cmd_id, _in_len);
                break;
            }
            case SOMEIP_SUBSCRIBE:
            case SOMEIP_UNSUBSCRIBE:
            {
                _msg = std::make_shared<AppMsgSubscribeCmd>(_cmd_id);
                break;
            }
            case SOMEIP_AVAILABLE_SERVICE:
            {
                _msg = std::make_shared<AppAvailableMsg>();
                break;
            }
            case SOMEIP_SEND:
            {
                _msg = std::make_shared<AppMsgSomeipSendCmd>(_in_len);
                break;
            }
            case SOMEIP_PING:
            {
                _msg = std::make_shared<AppMsgPing>();
                break;
            }
            case SOMEIP_PONG:
            {
                _msg = std::make_shared<AppMsgPong>();
                break;
            }
            case SOMEIP_UPPERTESTERCMD:
            {
                _msg = std::make_shared<AppMsgUpperTerter>();
                break;
            }
            case SOMEIP_REQUEST_EVENT_ACK:
            {
                _msg = std::make_shared<AppMsgEventAckCmd>(_in_len);
                break;
            }
            /* Start [GAC SPEC] upload Major version / interface version error to APP*/
            case SOMEIP_EXP_MAJOR_VER:
            {
                _msg = std::make_shared<ExpMajorVersionCmd>(_in_len);
                break;
            }
            /* End [GAC SPEC] upload Major version / interface version error to APP*/
            /*Start [GAC SPEC] request get service status */
            case SOMEIP_SERVICE_STATUS_REQ:
            {
                _msg = std::make_shared<AppMsgRequestGetServiceStatus>();
                break;
            }
            case SOMEIP_SERVICE_STATUS_RSP:
            {
                _msg = std::make_shared<AppMsgGetServiceStatusRsp>(_in_len);
                break;
            }
            /*End [GAC SPEC] request get service status */
            default:
                ret = false;
                break;
            }
            /* check input message len is equal to the special app message or not*/
            if (_msg && _msg->getMessageLen() != _in_len) {
                ret = false;
            }
            return ret;
        }
    };
}

#endif
