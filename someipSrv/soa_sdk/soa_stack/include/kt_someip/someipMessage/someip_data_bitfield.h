#ifndef _SOMEIP_DATA_BITFIELD_H_
#define _SOMEIP_DATA_BITFIELD_H_

#include "APF/ara/core/string_view.h"
#include "APF/Serializer.h"
#include "APF/ara/core/map.h"

namespace apf {
namespace someip {

struct BitFieldValue
{
	uint8_t offset;
	uint8_t length;
	uint32_t value = 0;
};

//template <typename T>
class BitField {
public:
	void addBitField(ara::core::StringView _name, uint8_t _offset, uint8_t _length)
	{
		BitFieldValue tempbitfield;
		tempbitfield.offset = _offset;
		tempbitfield.length = _length;
		m_data.insert(std::pair<ara::core::StringView, BitFieldValue>(_name, tempbitfield));
	}

	uint32_t getBitFieldByName(ara::core::StringView _name) {
		return m_data[_name].value;
	}

	void setBitFieldByName(ara::core::StringView _name, uint32_t data) {
		m_data[_name].value = data;
	}

	void setBufferSize()
	{
		m_buffer.clear();
		uint8_t datalength = 0;
		for (auto iter = m_data.begin(); iter != m_data.end(); iter++)
		{
			uint8_t templength = iter->second.offset + iter->second.length;
			datalength = templength > datalength ? templength : datalength;
		}
		datalength = datalength % 8 ? (datalength / 8) + 1 : datalength / 8;
		m_buffer.resize(datalength);
	}

	void setBitFieldBufferbyDataValue()
	{
		setBufferSize();
		for (auto iter = m_data.begin(); iter != m_data.end(); iter++)
		{
			uint8_t bytecount = iter->second.offset / 8;
			uint8_t bitcount = iter->second.offset % 8;
			uint8_t len = (bitcount == 0) ? iter->second.length / 8 : (iter->second.length+ (7 - bitcount)) / 8;
			uint32_t tempvalue = iter->second.value << (32 - bitcount - iter->second.length);
			uint8_t* target_addr = reinterpret_cast<uint8_t*>(&tempvalue);
			for (uint8_t i = 0; i <= len; i++)
			{
				m_buffer[bytecount + i] = m_buffer[bytecount + i] | target_addr[(sizeof(uint32_t)-1)-i];
			}
		}
	}

	void getDataValuefromBitFieldBuffer()
	{
		for (auto iter = m_data.begin(); iter != m_data.end(); iter++)
		{
			uint8_t bytecount = iter->second.offset / 8;
			uint8_t bitcount = iter->second.offset % 8;
			uint8_t len = (bitcount == 0) ? iter->second.length / 8 : (iter->second.length + (7 - bitcount)) / 8;
			uint32_t tempvalue;
			uint8_t* target_addr = reinterpret_cast<uint8_t*>(&tempvalue);
			for (uint8_t i = 0; i <= len; i++)
			{
				target_addr[(sizeof(uint32_t) - 1) - i] = m_buffer[bytecount + i];
			}
			tempvalue = *reinterpret_cast<uint32_t*>(target_addr);
			tempvalue = tempvalue << bitcount;
			tempvalue = tempvalue >> (32 - iter->second.length);
			iter->second.value = tempvalue;
		}
	}

	friend apf::Deserializer& operator >> (apf::Deserializer& _inputBuffer, BitField& _data) {
		_data.setBufferSize();
		for(size_t i = 0;i < _data.m_buffer.size();i++)
		{
			_inputBuffer >> _data.m_buffer[i];

		}
		_data.getDataValuefromBitFieldBuffer();
		return _inputBuffer;
	}

	friend apf::Serializer& operator << (apf::Serializer& _outputbuffer, BitField & _data) {
		_data.setBitFieldBufferbyDataValue();
		for (auto data : _data.m_buffer)
		{
			_outputbuffer << data;
		}
		return _outputbuffer;
	}

private:
	ara::core::Map<ara::core::StringView, BitFieldValue> m_data;
	ara::core::Vector<uint8_t> m_buffer;
};

}
}

#endif