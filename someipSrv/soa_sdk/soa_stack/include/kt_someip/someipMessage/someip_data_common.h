#ifndef _SOMEIP_DATA_COMMON_H_
#define _SOMEIP_DATA_COMMON_H_

#include <cstdint>
#include <vector>

#include "APF/Serializer.h"

namespace apf {
namespace someip {

enum class Alignment : size_t {
    kAlign8Bit = 1,
    kAlign16Bit = 2,
    kAlign32Bit = 4,
    kAlign64Bit = 8,
    kAlign128Bit = 16,
    kAlign256Bit = 32
};

class Configuration {
public:

    Configuration() :
        m_alignment(Alignment::kAlign32Bit),
        m_paddingdata(0)
    {}

    static Configuration& GetInstance()
    {
        static Configuration fc;
        return fc;
    }

    void setAlignment(Alignment _alignsize) { m_alignment = _alignsize; }
    Alignment getAlignment() { return m_alignment; }
    void setPaddingData(uint8_t _paddingdata) { m_paddingdata = _paddingdata; }
    uint8_t getPddingData() { return m_paddingdata; }

private:
    Alignment m_alignment;
    uint8_t m_paddingdata;
};

class Common {
public:
    static Common& GetInstance()
    {
        static Common fc;
        return fc;
    }

    Deserializer& skipPadding(Deserializer& _inputBuffer)
    {
        size_t deserializedLength = _inputBuffer.m_current_pos - _inputBuffer.m_buff.begin();
        size_t alignment = static_cast<size_t>(Configuration::GetInstance().getAlignment());
        if (deserializedLength % alignment != 0)
        {
            int skipsize = static_cast<int>(alignment - (deserializedLength % alignment));
            _inputBuffer.seekg(skipsize, _inputBuffer, SEEK_BUFFER::BUFFER_CUR);
        }
        return _inputBuffer;
    }

    Serializer& addPadding(Serializer& _outputBuff)
    {
        size_t currentlength = _outputBuff.getSize();
        size_t alignment = static_cast<size_t>(Configuration::GetInstance().getAlignment());
        if (currentlength % alignment != 0)
        {
            ara::core::Vector<uint8_t> padding;
            padding.resize(alignment - (currentlength % alignment), Configuration::GetInstance().getPddingData());
            _outputBuff << padding;
        }
        return _outputBuff;
    }

     /**
     * @fn       setLengthByOffset
     * @brief    Rewrite the length field by the offset after serialization.
     * @details  1.Save the offset of the complex data length field before serialization.
     *           2.Calculate the actual length according to the byte stream difference before and after serialization.
     *           3.Rewrite the length field by the offset after serialization.
     * @param    [in]_outputBuff : Serialized byte stream.
     *           [in]_offset : The offset of the length field.
     *           [in]_length : Actual length of String, struct, array, etc.
     *           [in]_endianType : Big endian and little endian for length field.
     */
    template <typename L = uint32_t, EndianType E = EndianType::kBigEndian>
    void setLengthByOffset(Serializer& _outputBuff, size_t _offset, apf::BinInt<L,E>& _length)
    {
        _length = static_cast<L>(_outputBuff.getSize() - _offset - sizeof(L));
        uint8_t* target_addr = reinterpret_cast<uint8_t*>(&_length);
        for (size_t i = 0; i < sizeof(L); i++)
        {
            if (E == EndianType::kLittleEndian) {
                _outputBuff.m_buffer[_offset + i] = target_addr[i];
            }
            else {
                _outputBuff.m_buffer[_offset + (sizeof(L) - 1 - i)] = target_addr[i];
            }          
        }
    }
};

class Bool
{
public:
    friend Serializer& operator << (Serializer& output_buffer, Bool const& _data) {
        output_buffer << _data.m_value;
        return output_buffer;
    }

    friend Deserializer& operator >> (Deserializer& input_buffer, Bool& _data) {
        uint8_t tempdata;
        input_buffer >> tempdata;
        _data = tempdata;

        return input_buffer;
    }

    Bool()
    {
        m_value = false;
    }

    template <typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
    Bool(T _value = 0)
    {
        m_value = _value & 0x01;
    }

    Bool(const Bool &_value)
    {
        m_value = _value.m_value;
    }

    operator bool& () {
        return m_value;
    }

    operator bool const& () const {
        return m_value;
    }

    template <typename T,  typename = typename std::enable_if<std::is_integral<T>::value>::type>
    Bool& operator = (T _value) {
        m_value = _value & 0x01;
        return *this;
    }

    Bool& operator = (Bool _value) {
        m_value = _value.m_value;
        return *this;
    }
private:
    bool m_value;
};

}//someip
}//apf
#endif