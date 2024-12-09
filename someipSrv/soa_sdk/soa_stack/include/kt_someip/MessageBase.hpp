/**
 * @file        MessageBase.hpp
 * @author      Jingwei Wang
 * @date        2022-03-24
 * @brief       MessageBase
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
#ifndef KT_SOMEIP_MESSAGEBASE_HPP
#define KT_SOMEIP_MESSAGEBASE_HPP
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include <ctime>
#include <vector>
#include <cstring>
#include "types.hpp"

namespace kt_someip
{
    /**
    *@brief message type in ktsomeipd
    */
    typedef enum _msgType{
        MSG_TYPE_INVALID = 0,
        MSG_TYPE_SD,
        MSG_TYPE_SOMEIP,
        MSG_TYPE_APP,
        MSG_TYPE_INNER,
        MSG_TYPE_MAX,
    }MsgType;

    /**
    *@brief MessageBase class is base class to all messages in ktsomeipd
    */
    class MessageBase{
    public:
        /**
        * MessageBase constructor
        * @param[in] T.B.D
        */
        MessageBase(size_t _len, MsgType _type = MSG_TYPE_INVALID)
            :m_type(_type),
            m_len(_len)
        {
            m_data.resize(_len, 0);
            m_timestamp = clock();
        }

        /**
        * MessageBase destructor
        */
        ~MessageBase() {
        }

        /**
        * serialize the head struct into data
        * @return true is success, false is failed
        */
        virtual bool serialize() = 0;

        /**
        * unserialize the head struct from data
        * @return true is success, false is failed
        */
        virtual bool unserialize() = 0;

        /**
        * get message type
        * @return MsgType
        */
        MsgType getMessageType(){return m_type;}

        /**
        * get message length according to data
        * @return message length
        */
        size_t getMessageLen() { return m_len; }

        /**
        * get member parameter: m_data
        * @return message length
        */
        std::vector<BYTE>& getMessageData() { return m_data; }

        /**
        * set message sirealized data
        * @param[in] sirealized byte array, copy socket message to messages
        */
        bool setMessageData(const std::vector<BYTE>& _data, size_t _len) {
            if (_len != m_len) {
                return false;
            }
            std::copy(_data.begin(), _data.begin() + _len, m_data.begin());
            return unserialize();
        }

        /**
        * get message create clock
        * @return clock_t
        */
        const clock_t& getMsgClock(){return m_timestamp;}

    protected:

        void setMessageLen(size_t _len) { 
            m_len = _len; 
            m_data.resize(_len, 0);
        }
        std::vector<BYTE> m_data;
    private:
        MsgType m_type;
        size_t  m_len;
        clock_t m_timestamp;
    };
}

#endif
