#ifndef _SOMEIP_MESSAGE_H_
#define _SOMEIP_MESSAGE_H_

#include <cstdint>
#include <vector>

#include "APF/ara/core/span.h"
#include "APF/ara/core/result.h"
#include "APF/UtilityTools.h"
#include "someip_data_common.h"

namespace apf { 
namespace someip {


const uint8_t kMsgHeaderLengthSize = 4U;
const uint8_t kMsgHeaderReqidSize = 4U;
const uint8_t kMsgHeaderLengthOffset = 4U;

// 
class MessageStandardHeader {
public:

    enum class SessionIdStatus {
        NotActive = 0,
        Active,
    };

    enum class MessageType : uint8_t {
        Request = 0x00,
        RequestNoReturn = 0x01,
        Notification = 0x02,
        Response = 0x80,
        Error = 0x81,
        TpRequest = 0x20,
        TpRequestNoReturn = 0x21,
        TpNotification = 0x22,
        TpResponse = 0xa0,
        TpError = 0xa1,
    };

    enum class ReturnCode : uint8_t {
        E_OK = 0x00,
        E_NOT_OK = 0x01,
        E_UNKNOWN_SERVICE = 0x02,
        E_UNKNOWN_METHOD = 0x03,
        E_NOT_READY = 0x04,
        E_NOT_REACHABLE = 0x05,
        E_TIMEOUT = 0x06,
        E_WRONG_PROTOCOL_VERSION = 0x07,
        E_WRONG_INTERFACE_VERSION = 0x08,
        E_MALFORMED_MESSAGE = 0x09,
        E_WRONG_MESSAGE_TYPE = 0x0a,
        E_E2E_REPEATED = 0x0b,
        E_E2E_WRONG_SEQUENCE = 0x0c,
        E_E2E = 0x0d,
        E_E2E_NOT_AVAILABLE = 0x0e,
        E_E2E_NO_NEW_DATA = 0x0f,
    };

    enum class Err {
        kWrongMessageType = 0,

    };

    template <typename T>
    using Result = ara::core::Result<T, Err>;
    
public:

    MessageStandardHeader() :
    m_length(8),
    m_message_id(0),
    m_request_id(0),
    m_protocol_version(1U),
    m_interface_version(0),
    m_message_type(MessageType::Request),
    m_return_code(ReturnCode::E_OK) {}

    void setMessageId(uint32_t _message_id) {
        m_message_id = _message_id;
    }

    void setServiceId(uint16_t _service_id) {
        m_message_id = (m_message_id & 0x0000FFFF) | (_service_id << 16);
    }

    void setMethodId(uint16_t _method_id) {
        m_message_id = (m_message_id & 0xFFFF0000) | ((_method_id << 1) >> 1);
    }

    void setEventId(uint16_t _event_id) {
        m_message_id = (m_message_id & 0xFFFF0000) | (0x8000 | _event_id);
    }

    void setLength(uint32_t _length) {  // @todo ?? really need to be set by user??
        m_length = _length;
    }                         
    
    void setRequestId(uint32_t _request_id) {
        m_request_id = _request_id;
    }

    void setClientId(uint16_t _client_id) {
        m_request_id = (m_request_id & 0x0000FFFF) | (static_cast<uint32_t>(_client_id) << 16);
    }

    void setClientIdPrex(uint8_t _client_id_prex) {
        m_request_id = (m_request_id & 0x00FFFFFF) | (static_cast<uint32_t>(_client_id_prex) << 24);
    }

    void setSessionId(uint16_t _session_id) {
        m_request_id = (m_request_id & 0xFFFF0000) | _session_id;
    }

    void setProtocolVersion(uint8_t _protocol_version) {
        m_protocol_version = _protocol_version;
    } 

    void setInterfaceVersion(uint8_t _interface_version) {
        m_interface_version = _interface_version;
    }

    void setMessageType(MessageType _message_type) {
        m_message_type = _message_type;
    }

    void setReturnCode(ReturnCode _return_code) {
        m_return_code = _return_code;
    }

    uint32_t getMessageId() const {
        return m_message_id;
    }

    uint16_t getServiceId() const {
        return (m_message_id >> 16);
    }

    Result<uint16_t> getMethodId() const {
        return isMethodMessage() ? 
                Result<uint16_t>::FromValue(static_cast<uint16_t>(m_message_id & 0x00007FFF))
                : Result<uint16_t>::FromError(Err::kWrongMessageType);
    }

    Result<uint16_t> getEventId() const {
        return !isMethodMessage() ? 
                Result<uint16_t>::FromValue(static_cast<uint16_t>(m_message_id & 0x00007FFF))
                : Result<uint16_t>::FromError(Err::kWrongMessageType);
    }

    /* NG : for xinghe project, inconformity with someip protocol */
    uint16_t getMethodEventId() const {
        return (m_message_id << 16) >> 16;
    }

    uint32_t getLength() const {
        return m_length;
    }
    
    uint32_t getRequestId() const {
        return m_request_id;
    }

    uint16_t getClientId() const {
        return static_cast<uint16_t>(m_request_id >> 16);
    }

    uint16_t getSessionId() const {
        return static_cast<uint16_t>(m_request_id & 0x0000FFFF);
    }

    SessionIdStatus getSessionIdStatus() const {
        return getSessionId() ? SessionIdStatus::Active : SessionIdStatus::NotActive;
    }

    uint8_t getProtocolVersion() const {
        return m_protocol_version;
    }

    uint8_t getInterfaceVersion() const {
        return m_interface_version;
    }

    MessageType getMessageType() const {
        return m_message_type;
    }

    ReturnCode getReturnCode() const {
        return m_return_code;
    } 

    friend apf::Deserializer & operator >> (apf::Deserializer & inputBuffer, MessageStandardHeader & data) {
        inputBuffer >> data.m_message_id;
        inputBuffer >> data.m_length;
        inputBuffer >> data.m_request_id;
        inputBuffer >> data.m_protocol_version;
        inputBuffer >> data.m_interface_version;
        inputBuffer >> data.m_message_type;
        inputBuffer >> data.m_return_code;
        
        return inputBuffer;
    }

    friend apf::Serializer & operator << (apf::Serializer & outputBuff, MessageStandardHeader const & _data) {
        outputBuff << _data.m_message_id;
        outputBuff << _data.m_length;
        outputBuff << _data.m_request_id;
        outputBuff << _data.m_protocol_version;
        outputBuff << _data.m_interface_version;
        outputBuff << _data.m_message_type;
        outputBuff << _data.m_return_code; 

        return outputBuff;
    }

private:
    bool isMethodMessage() const { 
        return (m_message_id & 0x00008000) == 0;
    }

public:
    uint32_t                m_length;

protected:
    uint32_t                m_message_id;
  
    uint32_t                m_request_id;
    uint8_t                 m_protocol_version;
    uint8_t                 m_interface_version;
    MessageType             m_message_type;
    ReturnCode              m_return_code;
};


template <typename MessageHeader = MessageStandardHeader, typename T = ara::core::Vector<uint8_t>>
class SomeIpMessage {

    enum class SomeipMsgStatus {
        SOMEIP_MSG_OK = 0,
        SOMEIP_TP_MSG_SIZE_ERR,

    };

public:
    SomeIpMessage() { };
    SomeIpMessage(MessageHeader _header, T _data) : m_message_header(_header), m_message_payload(_data) { };
    void setMessageHeader(MessageHeader const & _message_header) {
        m_message_header = _message_header;
    }

    void setMessagePayload(T const & _message_payload) {
        m_message_payload = _message_payload;
    }

    MessageHeader const & getMessageHeader(void) const{
        return m_message_header;
    }

    MessageHeader & getMessageHeader(void) {
        return m_message_header;
    }

    T const & getMessagePayload(void) const {
        return m_message_payload;
    }

    T & getMessagePayload(void) {
        return m_message_payload;
    } 

    friend apf::Deserializer & operator >> (apf::Deserializer & _input_buffer, SomeIpMessage & _data) {
        _input_buffer >> _data.m_message_header;
        _input_buffer >> _data.m_message_payload;

        return _input_buffer;
    }

    friend apf::Serializer & operator << (apf::Serializer & output_buff, SomeIpMessage & _data) {
        output_buff << _data.m_message_header;
        output_buff << _data.m_message_payload;
        
        apf::BinInt<uint32_t, apf::EndianType::kBigEndian> length;
        length = _data.m_message_header.m_length;
        Common::GetInstance().setLengthByOffset(output_buff, static_cast<size_t>(kMsgHeaderLengthOffset), length);
        
        return output_buff;
    }
    
private:
    MessageHeader   m_message_header;
    T               m_message_payload;
    SomeipMsgStatus m_someip_status;
};

template <typename E2EProtocol>
class E2EMessageHeader : public MessageStandardHeader {
public:
    E2EMessageHeader() : m_e2e_header(0) { };

    void setMsgE2EHeader(E2EProtocol _m_e2e_header) {
        m_e2e_header = _m_e2e_header;
    }

    E2EProtocol const & getMsgE2EHeader(void) const {
        return m_e2e_header;
    }

    friend apf::Deserializer & operator >> (apf::Deserializer & inputBuffer, E2EMessageHeader & data) {
        inputBuffer >> dynamic_cast<MessageStandardHeader&>(data);
        inputBuffer >> data.m_e2e_header;

        return inputBuffer;
    }
    friend apf::Serializer & operator << (apf::Serializer & outputBuff, E2EMessageHeader const & data) {
        outputBuff << dynamic_cast<const MessageStandardHeader&>(data);
        outputBuff << data.m_e2e_header;

        return outputBuff;
    }


private:
    E2EProtocol   m_e2e_header;
}; 

class TPMessageHeader : public MessageStandardHeader {
    
public:
    TPMessageHeader() :m_tp_header(0) { };

    friend apf::Deserializer & operator >> (apf::Deserializer & inputBuffer, TPMessageHeader & data) {
        inputBuffer >> dynamic_cast<MessageStandardHeader&>(data);
        inputBuffer >> data.m_tp_header;

        return inputBuffer;
    }
    friend apf::Serializer & operator << (apf::Serializer & outputBuff, TPMessageHeader const & data) {
        outputBuff << dynamic_cast<const MessageStandardHeader&>(data);
        outputBuff << data.m_tp_header;

        return outputBuff;
    }

private:
    uint32_t m_tp_header;
    uint32_t m_tp_offset;
    uint8_t m_reserved;
    uint8_t m_more_segments_flag;
};

} // namespace someip
} // namespace apf


#endif