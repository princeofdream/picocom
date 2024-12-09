#ifndef _SERIALIZABLE_ARGUMENTS_H_
#define _SERIALIZABLE_ARGUMENTS_H_

#include "Serializer.h"

namespace apf {
namespace someip {

template < typename... Arguments_ >
struct SerializableArguments;

template < typename ArgumentType_ >
struct SerializableArguments< ArgumentType_ > {
    static inline bool Serializer(apf::Serializer& _outputBuff, /*const*/ ArgumentType_& _argument) {
        _outputBuff << _argument;
        return _outputBuff.isStatusOK();
    }

    static inline bool Deserializer(apf::Deserializer& _inputBuffer, ArgumentType_& _argument) {
        _inputBuffer >> _argument;
        return _inputBuffer.IsStatusOK() && (_inputBuffer.m_current_pos == _inputBuffer.m_buff.end());
    }
};

template < typename ArgumentType_, typename ... Rest_ >
struct SerializableArguments< ArgumentType_, Rest_... > {
    static inline bool Serializer(apf::Serializer& _outputBuff, /*const*/ ArgumentType_& _argument, /*const*/ Rest_ &... _rest) {
        _outputBuff << _argument;
        bool success = _outputBuff.isStatusOK();
        return success ? SerializableArguments<Rest_...>::Serializer(_outputBuff, _rest...) : false;
    }

    static inline bool Deserializer(apf::Deserializer& _inputBuffer, ArgumentType_& _argument, Rest_ &... _rest) {
        _inputBuffer >> _argument;
        bool success = _inputBuffer.IsStatusOK();
        return success ? SerializableArguments<Rest_...>::Deserializer(_inputBuffer, _rest...) : false;
    }
};

} // namespace someip
} // namespace apf

#endif // _SERIALIZABLE_ARGUMENTS_H_