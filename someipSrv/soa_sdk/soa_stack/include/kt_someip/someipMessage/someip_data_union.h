#ifndef _SOMEIP_DATA_UNION_H_
#define _SOMEIP_DATA_UNION_H_

#include "someip_data_common.h"
#include "APF/ara/core/variant.h"

namespace apf {
namespace someip {

template <class... _Types>
using Variant = ara::core::Variant<_Types...>;

enum class UnionErr {
    kUnion_OK = 0,
    kUnion_data_not_match,
};

template <typename T>
using UnionResult = ara::core::Result<T, UnionErr>;

template <typename D = uint32_t>
class UnionDataInfoNoL
{
public:
    D m_datatype;
    size_t m_curpos;

    void setDataType(D _datatype) { m_datatype = _datatype; };
    void setCurPos(size_t _curpos) { m_curpos = _curpos; }

    D getDatatype() { return m_datatype; }
    size_t getCurPos() { return m_curpos; }

    friend Deserializer& operator >> (Deserializer& _input_buffer, UnionDataInfoNoL& _data) {
        _input_buffer >> _data.m_datatype;
        return _input_buffer;
    }

    friend Serializer& operator << (Serializer& output_buffer, UnionDataInfoNoL& _data) {
        output_buffer << _data.m_datatype;
        return output_buffer;
    }
};

template <typename L = uint32_t,typename D = uint32_t,size_t ALIGN = 1>
class UnionDataInfo
{
public:
    D m_datatype;
    L m_length = 0;
    size_t m_curpos;

    void setDataType(D _datatype) { m_datatype = _datatype; };
    void setCurPos(size_t _curpos) { m_curpos = _curpos; }
    void setLength(size_t _length,bool _isTypefieldCovered = false) 
    { 
        if (_length % ALIGN != 0)
        {
            _length += ALIGN - _length % ALIGN;
        }
        m_length = static_cast<L>(_length); 
        if (_isTypefieldCovered == true)
        {
            m_length += sizeof(D);
        }
    }

    D getDatatype() { return m_datatype; }
    L getLength() { return m_length; }
    size_t getCurPos() { return m_curpos; }
    
    friend Deserializer& operator >> (Deserializer& _input_buffer, UnionDataInfo& _data) {
        _input_buffer >> _data.m_length;
        _input_buffer >> _data.m_datatype;
        return _input_buffer;
    }

    friend Serializer& operator << (Serializer& output_buffer, UnionDataInfo& _data) {
        output_buffer << _data.m_length;
        output_buffer << _data.m_datatype;
        return output_buffer;
    }
};

} // namespace someip
} // namespace apf
#endif //#ifndef _SOMEIP_DATA_UNION_H_