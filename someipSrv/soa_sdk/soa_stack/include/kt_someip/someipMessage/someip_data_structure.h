#ifndef _SOMEIP_DATA_STRUCTURE_H_
#define _SOMEIP_DATA_STRUCTURE_H_
#include <cstdint>
#include <vector>

#include "someip_data_common.h"
#include "someip_data_string.h"
#include "someip_data_array.h"
#include "someip_data_union.h"

namespace apf {
namespace someip {

template <typename L, EndianType E = EndianType::kBigEndian>
class Struct
{
public:
	Struct() {
		m_length = 0; 
	}

	virtual L getSize() = 0;

	/* SOMEIP_ID:	[PRS_SOMEIP_00900]
	 * Requirement:	If the length is less than the sum of the lengths of all struct members,
					and no substitution for the missing data can be provided locally by the receiver, 
					the deserialization shall be abortedand the message shall be treated as malformed.	
	*/
	bool isBufferEnough(apf::Deserializer& _inputBuffer)
	{
		if (m_length < getSize())
		{
			//[PRS_SOMEIP_00900]
			_inputBuffer.m_stream_status = apf::Deserializer::StreamStatus::OUT_RANGE;
			return false;
		}
		//Save the current position before deserialization.
		m_start_pos = _inputBuffer.m_current_pos;

		return true;
	}

	/* SOMEIP_ID:	[PRS_SOMEIP_00371]
	 * Requirement: If the length is greater than sum of the lengths of all struct members, 
					only the bytes specified in the data type shall be interpreted,
					and the other bytes shall be skipped based on the length field.
	*/
	void skipByteStream(apf::Deserializer& _inputBuffer)
	{
		if (m_length >= static_cast<L>(_inputBuffer.m_current_pos - m_start_pos)) {
			int len = ((m_length) - (_inputBuffer.m_current_pos - m_start_pos));
			_inputBuffer.seekg(len, _inputBuffer, apf::SEEK_BUFFER::BUFFER_CUR);
		}
		else {
			_inputBuffer.m_stream_status = apf::Deserializer::StreamStatus::BUFFER_OVER_FLOW;
		}
	}

public :
	apf::BinInt<L,E> m_length;
	ara::core::Vector<uint8_t>::iterator m_start_pos;
};

} // namespace someip
} // namespace apf
#endif	// #ifndef _SOMEIP_DATA_STRUCTURE_H_