#ifndef _SOMEIP_DATA_ARRAY_H_
#define _SOMEIP_DATA_ARRAY_H_

#include "someip_data_common.h"

namespace apf {
namespace someip {

enum class ArrayErr {
    kArray_OK = 0,
    kArray_data_not_match,
};

template <typename T, typename L, EndianType E>
class DynamicSizeArray {
private:
    ara::core::Vector<T> m_array;
    BinInt<L, E> m_length;
    L m_max_length = 0;

public:
    DynamicSizeArray(L _max_size = 0) { 
        m_max_length = _max_size; 
        m_length = apf::utility::safeIntCast<L>(m_array.size());
    }

    DynamicSizeArray(ara::core::Vector<T> _array, L _max_size = 0) {
        m_max_length = _max_size;
        setValue(_array);
    }

    ArrayErr setValue(ara::core::Vector<T>& _array)
    {
        m_length = apf::utility::safeIntCast<L>(_array.size());
        if ((m_length > m_max_length) && (m_max_length != 0))
        {
            m_array.clear();
            return ArrayErr::kArray_data_not_match;
        }
        m_array = _array;
        return ArrayErr::kArray_OK;
    }

    ara::core::Vector<T>& getValue() { return m_array; }

    // @brief >> operator for SizeBuff
    // template <class T1, class F1 = ara::core::Vector<uint8_t>>
    friend Deserializer& operator >> (Deserializer& _input_buffer, DynamicSizeArray& _data) {
        _input_buffer >> _data.m_length;
        if (_data.m_length > apf::utility::safeIntCast<L>(_input_buffer.m_buff.end() - _input_buffer.m_current_pos))
        {
            _input_buffer.m_stream_status = Deserializer::StreamStatus::OUT_RANGE;
            return _input_buffer;
        }

        auto start_pos = _input_buffer.m_current_pos;
        L deserialized_length = 0;
        while (deserialized_length < _data.m_length) {
            if (!_input_buffer.IsStatusOK()) {
                return _input_buffer;
            }
            _data.m_array.emplace_back(_input_buffer.read<T>().ValueOr(T()));
            deserialized_length = _input_buffer.m_current_pos - start_pos;
        }

        if (_data.m_array.size() > _data.m_max_length && _data.m_max_length != 0) {
            _data.m_array.erase(_data.m_array.begin() + _data.m_max_length, _data.m_array.end());
        }

        return _input_buffer;
    }

    // @brief >> operator for SizeBuff
    // template <class T2, class F2 = ara::core::Vector<uint8_t>>
    friend Serializer& operator << (Serializer& _outputBuffer, DynamicSizeArray& _data) {
        /* Save the offset of the Array length field before serialization. */
        size_t offset = _outputBuffer.getSize();
        _outputBuffer << _data.m_length;

        for (auto& item : _data.m_array) {
            _outputBuffer << item;
        }

        Common::GetInstance().setLengthByOffset(_outputBuffer, offset, _data.m_length);

        return _outputBuffer;
    }
};

template <typename T, size_t N, typename L, EndianType E>
class FixSizeArray {
private:
    BinInt<L, E> m_length;
    ara::core::Array<T, N> m_array;

public:
    FixSizeArray() {
        // @todo change to compile time calculation
        m_length = apf::utility::safeIntCast<L>(m_array.size()); 
    }

    FixSizeArray(ara::core::Array<T, N> _array) {
        // @todo change to compile time calculation
        m_length = apf::utility::safeIntCast<L>(m_array.size());
        m_array = _array;
    }

    // @todo change to compile time calculation
    L const& getArrayLength() const { return m_length; }

    void setValue(ara::core::Array<T, N>& _array) { m_array = _array; }
    ara::core::Array<T, N> const& getValue() const { return m_array; }

public:
    friend Deserializer& operator >> (Deserializer& _inputBuffer, FixSizeArray& _data) {
        _inputBuffer >> _data.m_length;
        if (_data.m_length > apf::utility::safeIntCast<L>(_inputBuffer.m_buff.end() - _inputBuffer.m_current_pos))
        {
            _inputBuffer.m_stream_status = Deserializer::StreamStatus::OUT_RANGE;
            return _inputBuffer;
        }

        auto start_pos = _inputBuffer.m_current_pos;
        L deserialize_length = 0;
        size_t i = 0;
        while ((deserialize_length < _data.m_length) && (i < N)) {
            _inputBuffer >> _data.m_array[i];
            deserialize_length = _inputBuffer.m_current_pos - start_pos;
            i += 1;
        }

        if (_data.m_length > deserialize_length)
        {
            _inputBuffer.m_current_pos = start_pos + apf::utility::safeIntCast<size_t>(_data.m_length);
        }
        return _inputBuffer;
    }

    friend Serializer& operator << (Serializer& _outputBuffer, FixSizeArray& _data) {
        /* Save the offset of the Array length field before serialization. */
        size_t offset = _outputBuffer.getSize();
        _outputBuffer << _data.m_length;

        for (auto& item : _data.m_array) {
            _outputBuffer << item;
        }

        Common::GetInstance().setLengthByOffset(_outputBuffer, offset, _data.m_length);

        return _outputBuffer;
    }
};

/*FixLenArray: fixed length array with length field
* T: Type of array data
* N: Size of array
* L: Type of length field, uint8_t/uint16_t/uint32_t, defult value uint32_t
* E: Big endian or little endian of length field
*/
template <typename T, size_t N, typename L = uint32_t, EndianType E = EndianType::kBigEndian>
using FixLenArray = FixSizeArray<T, N, L, E>;

/*DynLenArray: dynamic length array with length field
* T: Type of array data
* L: Type of length field, uint8_t/uint16_t/uint32_t, defult value uint32_t
* E: Big endian or little endian of length field
*/
template <typename T, typename L = uint32_t, EndianType E = EndianType::kBigEndian>
using DynLenArray = DynamicSizeArray<T, L, E>;

} //namespace someip
} //namespace apf

#endif //#ifndef _SOMEIP_DATA_ARRAY_H_