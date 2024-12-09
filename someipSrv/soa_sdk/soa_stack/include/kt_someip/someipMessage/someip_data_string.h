#ifndef _SOMEIP_DATA_STRING_H_
#define _SOMEIP_DATA_STRING_H_

#include <string>

#include "APF/TableDriveTool.h"
#include "someip_data_common.h"

namespace apf {
namespace someip {

enum class LengthStatus : uint8_t {
    kNormal = 0x00,
	kOverFlowError,	//The length of a string with fixed length is greater than expected
	kMissingError
};

/* FixLenString: String Length field is none
 * T: String data type: ara::core::String or ara::core::u16String
 * N: String expect length be based on data type definition(or max length)
 * E: String Endian: kBigEndian(UTF-8/UTF-16BE) or LittleEndian(UTF-16LE)
 */
template <typename T, uint32_t N, EndianType E = EndianType::kBigEndian>
class StringWithoutLen
{
public:
    StringWithoutLen() :
        m_is_padding(true)
    {}

    StringWithoutLen(T _str) :
        m_is_padding(true)
    {
        m_string = _str;
    }

    void operator = (T& _str) {
        m_string = _str;
    }
    
	/* SOMEIP_ID:	[PRS_SOMEIP_00087]
	 * Requirement:	String starts with BOM.
	 * BOM:			UTF-8:		EF BB BF   239 187 191
	 *				UTF-16BE:	FE FF      254 255
	 *				UTF-16LE:	FF FE      255 254
	 */
	
	void setBomByEncoding(apf::Deserializer& _inputBuffer)
	{
        if(typeid(T) == typeid(ara::core::String))
        {
            std::array <uint8_t, 0x03> bom; 
            _inputBuffer >> bom;
        }
        else
        {
            std::array <uint8_t, 0x02> bom;
            _inputBuffer >> bom;
        }
	}
	
    ara::core::Vector<uint8_t> getBomByEncoding()
    {
        std::pair<std::pair<bool, apf::EndianType>, ara::core::Vector<uint8_t>> table[] = {
            { { typeid(T) == typeid(ara::core::String),     apf::EndianType::kBigEndian     },  {0xEF, 0xBB, 0xBF}},
            { { typeid(T) == typeid(ara::core::u16String),  apf::EndianType::kBigEndian     },  {0xFE, 0xFF      }},
            { { typeid(T) == typeid(ara::core::u16String),  apf::EndianType::kLittleEndian  },  {0xFF, 0xFE      }},
        };
        ara::core::Vector<uint8_t> bom = MatchAndPickupItem_withDefault(table,
            (true == _item.first.first && E == _item.first.second), _item.second, table[0].second);

        return bom;
    }

    /* SOMEIP_ID:	[PRS_SOMEIP_00374]
     * Requirement:	The length of the string(this includes the "\0") in Bytes has to be specified in the data type definition.
                    Fill unused space using "\0".BOM is included in the length.
     */

    void setFixLenStrPadding(apf::Deserializer& _inputBuffer)
    {
        int32_t len_difference = (N - (m_string.length() + 1 + getBomByEncoding().size()));
        typeid(T) == typeid(ara::core::String) ? len_difference = len_difference : len_difference = len_difference * 2;

        if (len_difference > 0)
        {
            ara::core::Vector<uint8_t> padding(len_difference);
            for (int32_t i = 0; i < len_difference; i++)
            {
                _inputBuffer >> padding[i];
            }
        }
    }

    ara::core::Vector<uint8_t>& getFixLenStrPadding(ara::core::Vector<uint8_t>& _padding)
    {
        int32_t len_difference = (N - (m_string.length() + 1 + getBomByEncoding().size()));
        typeid(T) == typeid(ara::core::String) ? len_difference = len_difference : len_difference = len_difference * 2;

        if (len_difference > 0)
        {
            for (int32_t i = 0; i < len_difference; i++)
            {
                _padding.push_back(0);
            }
        }
        return _padding;
    }

    /* Function: Save serialized string.
     * Note :    Serialization fails when the length of the string exceeds the maximum length.
     */
        
    LengthStatus setStringValue(T& _str)
    {
        if (_str.length() + 1 + getBomByEncoding().size() > N)
        {
            m_string = _str.substr(0, N);
            return LengthStatus::kOverFlowError;
        }
        else
        {
            m_string = _str;
            return LengthStatus::kNormal;
        }
    }

    T const& getStringValue() const { return m_string; }

    size_t const& getSize() const { return N; }

    friend apf::Deserializer& operator >> (apf::Deserializer& _inputBuffer, StringWithoutLen& _data)
    {
        /* SOMEIP_ID:	[PRS_SOMEIP_00087]
         * Requirement:	String starts with BOM.
         */
        _data.setBomByEncoding(_inputBuffer);

        /* SOMEIP_ID:	[PRS_SOMEIP_00372]
         * Requirement:	String supports UTF-8/UTF-16BE/UTF-16LE encoding.
         */
        _inputBuffer >> _data.m_string;

        /* SOMEIP_ID:	[PRS_SOMEIP_00374]
         * Requirement:	The length of the string(this includes the "\0") in Bytes has to be specified in the data type definition.
                        Fill unused space using "\0".BOM is included in the length.
         */
        if (_data.m_is_padding == true) {
            _data.setFixLenStrPadding(_inputBuffer);
        }
        
        return _inputBuffer;
    }

    friend apf::Serializer& operator << (apf::Serializer& _outputBuff, StringWithoutLen& _data)
    {
        /* SOMEIP_ID:	[PRS_SOMEIP_00087]
         * Requirement:	String starts with BOM.
         */
        _outputBuff << _data.getBomByEncoding();

        /* SOMEIP_ID:	[PRS_SOMEIP_00372]
         * Requirement:	String supports UTF-8/UTF-16BE/UTF-16LE encoding.
         * SOMEIP_ID:	[PRS_SOMEIP_00373]
         * Requirement:	String ends with "\0".
         */
        _outputBuff << _data.getStringValue();

        /* SOMEIP_ID:	[PRS_SOMEIP_00374]
         * Requirement:	The length of the string(this includes the "\0") in Bytes has to be specified in the data type definition.
                        Fill unused space using "\0".BOM is included in the length.
         */
        if (_data.m_is_padding == true) {
            ara::core::Vector<uint8_t> padding;
            _outputBuff << _data.getFixLenStrPadding(padding);
        }

        return _outputBuff;
    }

protected:
    T       m_string;           //Save App's String data
    bool    m_is_padding;       //Set whether to need padding when the length of the string is less than the expected length  
};

/* DynLenString: String has length field
 * T: String data type: ara::core::String or ara::core::u16String
 * L: String length filed type: 8bit(uint8_t), 16bit(uint16_t) or 32bit(uint32_t)
 * N: String expect length be based on data type definition(or max length)
 * U: String Endian: kBigEndian(UTF-8/UTF-16BE) or LittleEndian(UTF-16LE)
 */
template <typename T, typename L = uint32_t, uint32_t N = 0xFFFFFFFF, EndianType E = EndianType::kBigEndian, typename = typename std::enable_if<std::is_integral<L>::value>::type>
class StringWithLen : public StringWithoutLen<T, N, E>
{
public:
    StringWithLen(bool _is_padding = false)
    {
        this->m_is_padding = _is_padding;
        m_length = N;
    }

    StringWithLen(T _str, bool _is_padding = false)
    {
        this->m_string = _str;
        this->m_is_padding = _is_padding;
        m_length = N;
    }
    
    L const& getSize() const { return m_length; }

    /* SOMEIP_ID:	[PRS_SOMEIP_00086][PRS_SOMEIP_00911][PRS_SOMEIP_00912][PRS_SOMEIP_00913][PRS_SOMEIP_00914]
     * Requirement:	Deserializer exception process.
     */
    LengthStatus exceptionProcess(apf::Deserializer& _inputBuffer, L _actual_len, L _expect_len)
    {
        typeid(T) == typeid(ara::core::String) ? _expect_len = _expect_len : _expect_len = _expect_len * 2 - 2;

        /* SOMEIP_ID:	[PRS_SOMEIP_00086]
         * Requirement:	UTF-16LE and UTF-16BE strings having an odd length the last byte shall be ignored.
                        The two bytes before shall be 0x00 bytes (termination).
         */
        auto ignoreLastByteOfOdd = [&]() {
            if (_actual_len % 2 == 1)
            {
                m_length = m_length - 1;
                _inputBuffer.m_buff.erase(_inputBuffer.m_buff.end() - 1);
            }
        };

        /* SOMEIP_ID:	[PRS_SOMEIP_00911]
         * Requirement:	If the length of a fixed length string is greater than expected,
                        the deserialization shall be aborted and the message shall be treated as malformed.
         */
        if ((_actual_len > _expect_len) || (_actual_len > static_cast<L>(_inputBuffer.m_buff.end() - _inputBuffer.m_current_pos)))
        {
            return LengthStatus::kOverFlowError;
        }

        /* SOMEIP_ID:	[PRS_SOMEIP_00912]
         * Requirement:	If the length of a fixed length string is less than expectedand and it is correctly terminated using "\0",
                        it shall be accepted.
         */
        if (_actual_len < _expect_len)
        {
            /* SOMEIP_ID:	[PRS_SOMEIP_00913]
             * Requirement:	If the length of a fixed length string is less than expected and it is not correctly terminated using "\0",
                            the deserialization shall be aborted and the message shall be treated as malformed.
             */
            ara::core::Vector<uint8_t>::iterator end_pos = _inputBuffer.m_current_pos;
            if (typeid(T) == typeid(ara::core::String))
            {
                end_pos = end_pos + m_length - 1;
                if (*(end_pos) != 0)
                {
                    return LengthStatus::kMissingError;
                }
            }
            else
            {
                ignoreLastByteOfOdd();
                end_pos = end_pos + m_length - 1;
                if ((*(end_pos) != 0) || (*(end_pos - 1) != 0))
                {
                    return LengthStatus::kMissingError;
                }
            }
        }
        return LengthStatus::kNormal;
    }

    friend apf::Deserializer& operator >> (apf::Deserializer& _inputBuffer, StringWithLen& _data)
    {
        /* SOMEIP_ID:	[PRS_SOMEIP_00089]
         * Requirement:	Dynamic length strings starts with a length field.
         */
        _inputBuffer >> _data.m_length;

        /* SOMEIP_ID:	[PRS_SOMEIP_00086][PRS_SOMEIP_00911][PRS_SOMEIP_00912][PRS_SOMEIP_00913][PRS_SOMEIP_00914]
         * Requirement:	Deserializer exception process.
         */
        if (_data.exceptionProcess(_inputBuffer, _data.m_length, static_cast<L>(N)) != LengthStatus::kNormal)
        {
            return _inputBuffer;
        }

        /* SOMEIP_ID:	[PRS_SOMEIP_00372]
         * Requirement:	String supports UTF-8/UTF-16BE/UTF-16LE encoding.
         */
        _inputBuffer >> dynamic_cast<StringWithoutLen<T, N, E>&>(_data);

        return _inputBuffer;
    }

    friend apf::Serializer& operator << (apf::Serializer& _outputBuff, StringWithLen& _data)
    {
        /* Save the offset of the String length field before serialization. */        
        auto offset = _outputBuff.getSize();

        /* SOMEIP_ID:	[PRS_SOMEIP_00089]
         * Requirement:	Dynamic length strings starts with a length field.
         */
        _outputBuff << _data.m_length;

        /* SOMEIP_ID:	[PRS_SOMEIP_00372]
         * Requirement:	String supports UTF-8/UTF-16BE/UTF-16LE encoding.
         */
        _outputBuff << dynamic_cast<StringWithoutLen<T, N, E>&>(_data);

        /* Calculate the actual length according to the bytes before and after serialization;
           Rewrite the length field by the offset after serialization.
         */
        Common::GetInstance().setLengthByOffset(_outputBuff, offset, _data.m_length);
             
        return _outputBuff;
    }
 private:
    BinInt<L, E>    m_length;    //Save length filed value
};
//============================================================

} //namespace someip
} //namespace apf
#endif //#ifndef _SOMEIP_DATA_STRING_H_