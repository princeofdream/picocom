#ifndef _ARA_COM_SERIALIZABLE_ARGUMENTS_HPP_
#define _ARA_COM_SERIALIZABLE_ARGUMENTS_HPP_

#include "OutputStream.hpp"
#include "InputStream.hpp"

namespace ara {
namespace com {

template < typename... Arguments_ >
struct SerializableArguments;

template <>
struct SerializableArguments<> {
    static inline bool serialize(OutputStream &) {
        return true;
    }

    static inline bool deserialize(InputStream &) {
        return true;
    }
};

template < typename ArgumentType_ >
struct SerializableArguments< ArgumentType_ > {
    static inline bool serialize(OutputStream &_output, const ArgumentType_ &_argument) {
        _output << _argument;
        return !_output.hasError();
    }

    static inline bool deserialize(InputStream &_input, ArgumentType_ &_argument) {
        _input >> _argument;
        return !_input.hasError();
    }
};

template < typename ArgumentType_, typename ... Rest_ >
struct SerializableArguments< ArgumentType_, Rest_... > {
    static inline bool serialize(OutputStream &_output, const ArgumentType_ &_argument, const Rest_ &... _rest) {
        _output << _argument;
        const bool success = !_output.hasError();
        return success ? SerializableArguments<Rest_...>::serialize(_output, _rest...) : false;
    }

    static inline bool deserialize(InputStream &_input, ArgumentType_ &_argument, Rest_ &... _rest) {
        _input >> _argument;
        const bool success = !_input.hasError();
        return success ? SerializableArguments<Rest_...>::deserialize(_input, _rest...) : false;
    }
};

} 
} 

#endif 
