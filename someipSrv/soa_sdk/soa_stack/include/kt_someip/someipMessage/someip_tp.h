#ifndef _SOMEIP_TP_H_
#define _SOMEIP_TP_H_

#include "APF/ara/core/map.h"
#include "someip_message.h"
#include "someip_data_array.h"

using namespace apf::someip;

namespace apf { 
namespace someip_tp {

const uint8_t kTpFlag = 0x20;
const uint8_t kMsgTpHeaderStdSize = 8U;
const uint8_t kMsgTpHeaderOffsetSize = 4U;

class SomeipTpHeader : public MessageStandardHeader {

public :
    enum class MoresegmentFlag : uint8_t{
        SOMEIP_TP_SEGMENTS_LAST = 0,
        SOMEIP_TP_SEGMENTS_NOTLAST,
    };

    //[PRS_SOMEIP_00726]
    SomeipTpHeader()  : m_someip_tp_header(0) { };
    SomeipTpHeader(MessageStandardHeader _standard_header) : MessageStandardHeader(_standard_header), m_someip_tp_header(0) {}

    /* TP offset : the value provided within the Offset Field is given in units of 16 bytes */
    void setSomeipTpOffset(uint32_t _offset) {
        uint32_t offset =  _offset / 16;
        m_someip_tp_header = (m_someip_tp_header & 0x0000000f) |  (offset << 4);
    }

    /* someiip_tp offset ：the value provided within the Offset Field is given in units of 16 bytes */
    uint32_t getSomeipTpOffset() const {
        return ((m_someip_tp_header & 0xfffffff0) >> 4);
    }

    /* The offset from the original data(some ip data)  */
    uint32_t getSomeipPayloadOffset() const {
        return ((m_someip_tp_header & 0xfffffff0) >> 4) * 16;
    }

    //[PRS_SOMEIP_00727]
    void setSomeipTpMoresegmentsFlag(bool _is_last_segment) {
        if (_is_last_segment){
            m_someip_tp_header = m_someip_tp_header | 0x00000001;
        }
        else {
            m_someip_tp_header = m_someip_tp_header & 0xfffffff0;
        }
    } // @todo ,very strange

    uint8_t getSomeipReservedFlag() const {
        return ((m_someip_tp_header & 0xf) >> 1);
    }

    MoresegmentFlag getSomeipTpMoresegmentsFlag() const {
        return (m_someip_tp_header & 0x00000001) ? MoresegmentFlag::SOMEIP_TP_SEGMENTS_NOTLAST : MoresegmentFlag::SOMEIP_TP_SEGMENTS_LAST;
    }

    void setSomeipTpHeader(uint32_t _tp_header) {
        m_someip_tp_header = _tp_header;
    }

    uint32_t getSomeipTpHeader() const {
        return m_someip_tp_header;
    }

    friend apf::Deserializer & operator >> (apf::Deserializer & inputBuffer, SomeipTpHeader & data) {
        inputBuffer >> dynamic_cast<MessageStandardHeader&>(data);
        inputBuffer >> data.m_someip_tp_header;

        return inputBuffer;
    }
    friend apf::Serializer & operator << (apf::Serializer & outputBuff, SomeipTpHeader const & data) {
        outputBuff << dynamic_cast<const MessageStandardHeader&>(data);
        outputBuff << data.m_someip_tp_header;

        return outputBuff;
    }

private :
    //[PRS_SOMEIP_00723]
    uint32_t m_someip_tp_header;
    
};

class SomeipTp {
public:
    enum class Err {
        SOMEIP_TP_OK = 0,
        TP_MSG_NO_DATA,
        TP_MSG_SET_DATA_ERR,
        TP_MSG_HEADER_UNMATH,
        TP_MSG_LENGTH_UNMATH,
        TP_MSG_SET_DATA_UNCOMPLETE,
        TP_MSG_OVERLAPPING,
        TP_MSG_MAXSIZE_ERR,

    };

    template <typename T>
    using Result = ara::core::Result<T, Err>;

public :
    SomeipTp() { };

    //split message
    Result<ara::core::Vector<SomeIpMessage<SomeipTpHeader>>> getSomeIpTpMsgSet(SomeIpMessage<> const & _message_std, uint32_t _max_message_size) const {
        //[PRS_SOMEIP_00729][PRS_SOMEIP_00730]
        if (!checkMaxMessageSizeValid(_max_message_size)) {
            return Err::TP_MSG_MAXSIZE_ERR;
        }

        //[PRS_SOMEIP_00720][PRS_SOMEIP_00721][PRS_SOMEIP_00731]
        MessageStandardHeader header = _message_std.getMessageHeader();
        //[PRS_SOMEIP_00722][PRS_SOMEIP_00367]
        header.setMessageType(static_cast<MessageStandardHeader::MessageType>(apf::utility::to_underlying(header.getMessageType()) | kTpFlag));

        uint32_t payload_size  = header.getLength() - kMsgHeaderLengthSize - kMsgHeaderReqidSize;
        uint32_t _remained_size = payload_size; // @todo check type 

        ara::core::Vector<uint8_t>::const_iterator iter_to_payload = _message_std.getMessagePayload().cbegin();    // @todo should use byte
        ara::core::Vector<SomeIpMessage<SomeipTpHeader>> rslt;
        while (_remained_size > 0) {
            //[PRS_SOMEIP_00729][PRS_SOMEIP_00734][PRS_SOMEIP_00735]
            auto this_payload_size = std::min(_remained_size, _max_message_size);

            //[PRS_SOMEIP_00720][PRS_SOMEIP_00721][PRS_SOMEIP_00731]
            SomeipTpHeader tp_header(header);
            //[PRS_SOMEIP_00725]
            tp_header.setSomeipTpOffset(payload_size - _remained_size);
            //[PRS_SOMEIP_00727]
            tp_header.setSomeipTpMoresegmentsFlag(_remained_size > _max_message_size);
            //[PRS_SOMEIP_00728]
            tp_header.setLength(this_payload_size + kMsgTpHeaderStdSize + kMsgTpHeaderOffsetSize);

            rslt.push_back(SomeIpMessage<SomeipTpHeader>());
            rslt.back().setMessageHeader(tp_header);
            uint32_t offset = tp_header.getSomeipPayloadOffset();
            rslt.back().setMessagePayload({iter_to_payload + offset, iter_to_payload + offset + this_payload_size});
            _remained_size = _remained_size - this_payload_size;
        }

        return rslt;
    }

    //reassemble message
    Result<SomeIpMessage<>> getSomeIpTpReassembleMessage(ara::core::Vector<SomeIpMessage<SomeipTpHeader>> const & _message_set) {
        auto msg_set_order = isTpMessageSetCompleted(_message_set);
        if (msg_set_order.HasValue() == false) {
            return msg_set_order.Error();
        }

        std::vector<uint8_t> msg_data; // @todo should use ara::core::Vector
        for (auto const & _msg : msg_set_order.Value()) { 
            uint32_t segment_size = _msg.getMessageHeader().getLength() - kMsgTpHeaderStdSize - kMsgTpHeaderOffsetSize;
            auto const& segment_data = _msg.getMessagePayload();
            msg_data.insert(msg_data.end(), segment_data.begin(), segment_data.begin() + segment_size);
        }

        //[PRS_SOMEIP_00745]
        MessageStandardHeader header_std = msg_set_order.Value().back().getMessageHeader();
        header_std.setLength(static_cast<uint32_t>(msg_data.size() + kMsgTpHeaderStdSize));
        //[PRS_SOMEIP_00746][PRS_SOMEIP_00367]
        header_std.setMessageType(static_cast<MessageStandardHeader::MessageType>(apf::utility::to_underlying(header_std.getMessageType()) & 0xdf));

        SomeIpMessage<> rslt;
        rslt.setMessageHeader(header_std);
        rslt.setMessagePayload(std::move(msg_data));

        return rslt;
    }

private :
    //[PRS_SOMEIP_00729][PRS_SOMEIP_00730]
    bool checkMaxMessageSizeValid(uint32_t _max_size) const {
        //[PRS_SOMEIP_00729][PRS_SOMEIP_00730]
        return ((_max_size % 16) == 0) && (_max_size <= 1392);
    }

    //[PRS_SOMEIP_00738]
    bool isSameSomeipTpHeader(SomeipTpHeader const & _header_1, SomeipTpHeader const & _header_2) const {
        return ((_header_1.getMessageId() == _header_2.getMessageId()) && 
            (_header_1.getRequestId() == _header_2.getRequestId()) && 
            (_header_1.getProtocolVersion() == _header_2.getProtocolVersion()) &&
            (_header_1.getInterfaceVersion() == _header_2.getInterfaceVersion()) &&
            (_header_1.getMessageType() == _header_2.getMessageType()) &&
            ((apf::utility::to_underlying(_header_1.getMessageType()) & 0x20) == 0x20)
            );
    }
    
    //[PRS_SOMEIP_00743]
    bool isTpMsgLengthMach(uint32_t _length, uint32_t _payload_size) const {
        return (_length <= (_payload_size + kMsgTpHeaderStdSize + kMsgTpHeaderOffsetSize));
    }

    //[PRS_SOMEIP_00744]
    Err checkMessageSetValid(ara::core::Vector<SomeIpMessage<SomeipTpHeader>> const & _message_set) const {
        if(0 == _message_set.size()){
            return Err::TP_MSG_NO_DATA;
        }

        SomeipTpHeader tp_header = _message_set[0].getMessageHeader();

        uint32_t data_size = 0;
        for (uint32_t i = 0; i < _message_set.size(); i++) {
            
            //check first mesegment's offset is 0
            if ((i == 0) && (_message_set[i].getMessageHeader().getSomeipTpOffset() != 0))
                return Err::TP_MSG_SET_DATA_ERR;

            //[PRS_SOMEIP_00738]
            //check segments's header is the same
            if (!isSameSomeipTpHeader(tp_header, _message_set[i].getMessageHeader()))
                return Err::TP_MSG_HEADER_UNMATH;

            //[PRS_SOMEIP_00743]
            //check length and data size is mach or not
            if (!isTpMsgLengthMach(_message_set[i].getMessageHeader().getLength(),
                static_cast<uint16_t>(_message_set[i].getMessagePayload().size())))
                return Err::TP_MSG_LENGTH_UNMATH;

            //check more segment flag is 1 when msg is not last
            if ((i < (_message_set.size() - 1)) && 
                (_message_set[i].getMessageHeader().getSomeipTpMoresegmentsFlag() != SomeipTpHeader::MoresegmentFlag::SOMEIP_TP_SEGMENTS_NOTLAST))
                return Err::TP_MSG_SET_DATA_ERR;

            //check more segment flag is 0 when msg is last
            if ((i == (_message_set.size() - 1)) && 
                (_message_set[i].getMessageHeader().getSomeipTpMoresegmentsFlag() != SomeipTpHeader::MoresegmentFlag::SOMEIP_TP_SEGMENTS_LAST))
                return Err::TP_MSG_SET_DATA_ERR; 

            //[PRS_SOMEIP_00754] 
            //check more segment flag = 1, length must a multiple of 16
            if ((_message_set[i].getMessageHeader().getSomeipTpMoresegmentsFlag() == SomeipTpHeader::MoresegmentFlag::SOMEIP_TP_SEGMENTS_NOTLAST) &&
                (((_message_set[i].getMessageHeader().getLength()  - kMsgTpHeaderStdSize - kMsgTpHeaderOffsetSize) % 16) != 0))
                return Err::TP_MSG_SET_DATA_ERR;

            //[PRS_SOMEIP_00749]
            //check offset and msg data is mach
            if ((i > 0) && (_message_set[i].getMessageHeader().getSomeipPayloadOffset() != data_size))
                return Err::TP_MSG_SET_DATA_ERR;

            data_size = data_size + (_message_set[i].getMessageHeader().getLength() - kMsgTpHeaderStdSize - kMsgTpHeaderOffsetSize);     
        }

        return Err::SOMEIP_TP_OK;
    }
    
    Result<ara::core::Vector<SomeIpMessage<SomeipTpHeader>>> isTpMessageSetCompleted(ara::core::Vector<SomeIpMessage<SomeipTpHeader>> const & _message_set) const {
        using segmentTypeData = std::pair<uint32_t, std::reference_wrapper<const SomeIpMessage<SomeipTpHeader>>>; // @todo add ara::core::Set to apf
        ara::core::Vector<segmentTypeData> segments;
        segments.reserve(_message_set.size());
        for(auto const& _msg : _message_set) {
            segments.push_back({_msg.getMessageHeader().getSomeipTpOffset(), std::cref(_msg)});
        }
        //[PRS_SOMEIP_00747][PRS_SOMEIP_00733]
        std::sort(segments.begin(), segments.end(), [](segmentTypeData const & _a, segmentTypeData const & _b) { return _a.first < _b.first; });

        ara::core::Vector<SomeIpMessage<SomeipTpHeader>> message_set_ordered;
        message_set_ordered.reserve(_message_set.size());  
        for (auto _msg : segments) {
            message_set_ordered.push_back(_msg.second.get());
        }
        
        //[PRS_SOMEIP_00744]
        auto msg_valid = checkMessageSetValid(message_set_ordered);
        if (Err::SOMEIP_TP_OK != msg_valid) {
            return msg_valid;
        }

        return message_set_ordered;
    }
};

class SomeIpTpMsgPackages : public SomeipTp {

public :
    SomeIpTpMsgPackages() {};
   
    ara::core::Map<uint16_t, ara::core::Vector<SomeIpMessage<SomeipTpHeader>>> & getTpRcvData(void) {
        return m_tp_msg_map;
    }

    bool isSameSomeipTpPackage(ara::core::Vector<uint8_t> const & _rcv_msg, ara::core::Vector<uint8_t> const & _last_msg ) {
        return (_rcv_msg == _last_msg);
    }

    Result<SomeIpMessage<>> TryPackages(ara::core::Vector<uint8_t> const & _rcv_msg) {
        apf::Deserializer dse(_rcv_msg);
        SomeIpMessage<SomeipTpHeader> tp_msg;
        dse >> tp_msg;

        //check clientId has or not
        uint16_t clent_id = tp_msg.getMessageHeader().getClientId();

        ara::core::Vector<SomeIpMessage<SomeipTpHeader>> m_tp_msg_set = m_tp_msg_map[clent_id];

        //[PRS_SOMEIP_00752][PRS_SOMEIP_00753]
        if (0 != m_tp_msg_set.size()) {
            //printf("*******%d********\n", m_tp_msg_set.size());
            apf::Serializer se;
            se << m_tp_msg_set.back();
            if (isSameSomeipTpPackage(_rcv_msg, se.m_buffer)) 
                return Err::TP_MSG_OVERLAPPING;
        }

        Result<SomeIpMessage<>> rslt = Err::SOMEIP_TP_OK;
        //[PRS_SOMEIP_00741][PRS_SOMEIP_00742][PRS_SOMEIP_00750]
        if (m_session_id[clent_id] != tp_msg.getMessageHeader().getSessionId()) {
            rslt = SomeipTp::getSomeIpTpReassembleMessage(m_tp_msg_set);
            ara::core::Vector<SomeIpMessage<SomeipTpHeader>>().swap(m_tp_msg_map[clent_id]);
            m_tp_msg_map.erase(clent_id);
            m_session_id[clent_id] = tp_msg.getMessageHeader().getSessionId();
        }
        else {
            rslt = Err::TP_MSG_SET_DATA_UNCOMPLETE;
        }
        m_tp_msg_map[clent_id].push_back(tp_msg);

        return rslt;        
    }


private :
    ara::core::Map<uint16_t, ara::core::Vector<SomeIpMessage<SomeipTpHeader>>> m_tp_msg_map;
    ara::core::Map<uint16_t, uint16_t> m_session_id;

};

}//someip
}//apf


#endif