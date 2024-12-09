/**
 * @file        AppMessage.hpp
 * @author      Jingwei Wang
 * @date        2022-03-29
 * @brief       AppMessage
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
#ifndef KT_SOMEIP_APPMESSAGE_HPP
#define KT_SOMEIP_APPMESSAGE_HPP
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <vector>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include "kt_someip/MessageBase.hpp"

namespace kt_someip
{
    #define APP_HEADER_LEN      15
    #define START_POS           0
    #define LEN_POS             4
    #define COMMAND_ID_POS      8
    #define APP_ID_POS          9
    #define RESERVE_POS         11
    #define PAYLOAD_POS         15

    #define START_MAGIC_NUM     (0x67376d07)

    /* Message length for below command:
        SOMEIP_SUBSCRIBE
        SOMEIP_UNSUBSCRIBE */
    #define APP_SUBSCRIBE_MSG_LEN          (APP_HEADER_LEN + 11)
    /* Message length for below command:
        SOMEIP_REGISTER_EVENT
        SOMEIP_UNREGISTER_EVENT
        SOMEIP_OFFER_EVENT
        SOMEIP_OFFERED_EVENT_ACK */
    #define APP_EVENT_MSG_LEN              (APP_HEADER_LEN + 10)
    /* Message length for below command:
        SOMEIP_OFFER_SERVICE
        SOMEIP_STOP_OFFER_SERVICE
        SOMEIP_REQUEST_SERVICE
        SOMEIP_RELEASE_SERVICE */
    #define APP_SERVICE_MSG_LEN            (APP_HEADER_LEN + 9)
    /* Message length for below command:
        SOMEIP_AVAILABLE_SERVICE*/
    #define APP_SERVICE_AVAILABLE_LEN      (APP_HEADER_LEN + 10)
    /* Message length for SOMEIP_REQUEST_APPID_ACK*/
    #define REQ_APP_ID_ACK_LEN             (APP_HEADER_LEN + 12)

    /* someipd -> APP commands*/
    #define	SOMEIP_REQUEST_APPID_ACK	            0x01
    #define	SOMEIP_PING                             0x02
    #define	SOMEIP_AVAILABLE_SERVICE	            0x03
    #define	SOMEIP_OFFERED_EVENT_ACK	            0x04
    #define	SOMEIP_REQUEST_EVENT_ACK   	            0x05
    /* Start [GAC SPEC] upload Major version / interface version error to APP*/
    #define	SOMEIP_EXP_MAJOR_VER  	                0x06
    /* End [GAC SPEC] upload Major version / interface version error to APP*/
    /*Start [GAC SPEC] service status response */
    #define	SOMEIP_SERVICE_STATUS_RSP               0x71
    /*End [GAC SPEC] service status response*/

    /* APP -> someipd commands*/
    #define	SOMEIP_REQUEST_APPID	                0x80
    #define	SOMEIP_DEREGISTER_APPLICATION	        0x81
    #define	SOMEIP_PONG	                            0x82
    #define	SOMEIP_OFFER_SERVICE	                0x83
    #define	SOMEIP_STOP_OFFER_SERVICE	            0x84
    #define	SOMEIP_REQUEST_SERVICE	                0x85
    #define	SOMEIP_RELEASE_SERVICE	                0x86
    #define	SOMEIP_REGISTER_EVENT	                0x88
    #define	SOMEIP_OFFER_EVENT	                    0x89
    #define	SOMEIP_STOP_OFFER_EVENT	                0x8A
    #define	SOMEIP_UNREGISTER_EVENT     	        0x9A
    #define	SOMEIP_SUBSCRIBE	                    0x9D
    #define	SOMEIP_UNSUBSCRIBE	                    0x9F
    #define	SOMEIP_UPPERTESTERCMD                   0xF0
    /*Start [GAC SPEC] service status response */
    #define	SOMEIP_SERVICE_STATUS_REQ               0xF2
    /*End [GAC SPEC] service status response*/

    /* APP <-> someipd commands*/
    #define	SOMEIP_SEND	                            0xA0

    /**
    *@brief AppMessage class is base class to all messages in ktsomeipd
    */
    class AppMessage : public MessageBase {
    public:

        /**
        *@brief message header
        */
        typedef struct _AppHeader {
            uint32_t start{ START_MAGIC_NUM };
            uint32_t len;
            uint8_t  commanId;
            uint16_t appId;
            uint32_t reseved;
        }AppHeader;

        /**
        * AppMessage constructor
        * @param[in] T.B.D
        */
        AppMessage(size_t _len) :MessageBase(_len, MSG_TYPE_APP) {
        }

        /**
        * AppMessage destructor
        */
        ~AppMessage() {}

        /**
        * setHeader
        * @param[in] header
        */
        void setHeader(AppHeader& _head) {
            m_header = _head;
            memcpy(&m_data[START_POS], &m_header.start,
                sizeof(m_header.start));
            memcpy(&m_data[LEN_POS], &m_header.len,
                sizeof(m_header.len));
            memcpy(&m_data[COMMAND_ID_POS], &m_header.commanId,
                sizeof(m_header.commanId));
            memcpy(&m_data[APP_ID_POS], &m_header.appId,
                sizeof(m_header.appId));
            memcpy(&m_data[RESERVE_POS], &m_header.reseved,
                sizeof(m_header.reseved));
        }

        /**
        * getHeader
        * @param[in] header
        */
        AppHeader& getHeader() { return m_header; }

        void setMagicStart() {
            m_header.start = START_MAGIC_NUM;
            memcpy(&m_data[START_POS], &m_header.start, sizeof(m_header.start));
        }
        /**
        * setHeaderCommandId
        * @param[in] header->commanId
        */
        void setCommandId(uint8_t _cId) {
            setMagicStart();
            m_header.commanId = _cId;
            memcpy(&m_data[COMMAND_ID_POS], &m_header.commanId, sizeof(m_header.commanId));
        }

        /**
        * getHeaderCommandId
        * @return header->commanId
        */
        uint8_t& getCommandId() { return m_header.commanId; }

        /**
        * setHeaderAppId
        * @param[in] header->appId
        */
        void setAppId(uint16_t _appId) {
            m_header.appId = _appId;
            memcpy(&m_data[APP_ID_POS], &m_header.appId, sizeof(m_header.appId));
        }

        /**
        * getHeaderAppId
        * @return header->appId
        */
        uint16_t& getAppId() { return m_header.appId; }

        /**
        * setPayloadLen
        * @param[in] header->len
        */
        void setPayloadLen(uint32_t _len) {
            m_header.len = _len;
            memcpy(&m_data[LEN_POS], &m_header.len, sizeof(m_header.len));
        }

        /**
        * getPayloadLen
        * @return header->len
        */
        uint32_t& getPayloadLen() { return m_header.len; }

        /**
        * setHeaderReseved
        * @param[in] header->reseved
        */
        void setReseved(uint32_t _res) {
            m_header.reseved = _res;
            memcpy(&m_data[RESERVE_POS], &m_header.reseved, sizeof(m_header.reseved));
        }

        /**
        * getHeaderReseved
        * @param[in] header->reseved
        */
        uint32_t& getReseved() { return m_header.reseved; }

        bool serialize() { return true; }
        /**
        * unserialize the head struct from data
        * @return true is success, false is failed
        */
        bool unserialize() {
            // data -> header
            memcpy(&m_header.start, &m_data[START_POS], sizeof(m_header.start));
            memcpy(&m_header.len, &m_data[LEN_POS], sizeof(m_header.len));
            memcpy(&m_header.commanId, &m_data[COMMAND_ID_POS], sizeof(m_header.commanId));
            memcpy(&m_header.appId, &m_data[APP_ID_POS], sizeof(m_header.appId));
            memcpy(&m_header.reseved, &m_data[RESERVE_POS], sizeof(m_header.reseved));
            unserializePayload();
            return true;
        }

        /**
        * unserialize payload
        * @return true is success, false is failed
        */
        virtual void unserializePayload() = 0;
        virtual uint16_t getSrvid() = 0;
        virtual uint16_t getInsid() = 0;
        virtual uint8_t getMajor() = 0;

        virtual std::string getCmdString() { return ""; };

    protected:
        AppHeader                       m_header;
    };
}

#endif
