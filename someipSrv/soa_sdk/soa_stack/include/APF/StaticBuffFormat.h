#ifndef __STATIC_BUFF_FORMAT_H__
#define __STATIC_BUFF_FORMAT_H__

#include <string>
#include "ara/core/string_view.h"
#include "StaticSStream.h"


namespace apf {
namespace str {

// Target type should be derived from CharBuffSStream
template<typename Target>
void __sFormat(Target& _ss, ara::core::StringView _format_str)
{
    _ss << _format_str;
}

// Target type should be derived from CharBuffSStream
template<typename Target, typename T, typename... Args>
void __sFormat(Target& _ss, ara::core::StringView _format_str, T&& value, Args &&... _args)
{
    auto pos = _format_str.find("{}");
    if (pos == ara::core::StringView::npos) {
        _ss << _format_str;
    }
    else {
        _ss << _format_str.substr(0, pos);
        _ss << value;
        _format_str.remove_prefix(pos + 2);
        if (_format_str.empty() == false) {
            __sFormat(_ss, _format_str, std::forward<Args>(_args)...);
        }
    }    
}

// Target type should be derived from CharBuffSStream
template<typename Target>
ara::core::StringView sFormat(Target& _ss, ara::core::StringView _format_str) {
    _ss << _format_str;
    return _ss.stringView();
}

// 
// Target type should be derived from CharBuffSStream
template<typename Target, typename T, typename... Args>
ara::core::StringView sFormat(Target& _ss, ara::core::StringView _format_str, T&& value, Args && ... _args) {
    __sFormat(_ss, _format_str, value, std::forward<Args>(_args)...);
    return _ss.stringView();
}

template< typename T, typename... Args>
ara::core::StringView sFormat(char * _buff, size_t _size, ara::core::StringView _format_str, T&& value, Args&& ... _args) {
    CharBuffSStream ss(_buff , _size);
    __sFormat(ss, _format_str, value, std::forward<Args>(_args)...);
    return ss.stringView();
}

// static buff format
template<size_t N, typename T, typename... Args>
ara::core::StringView sFormat(char(&_buff)[N], ara::core::StringView _format_str, T&& value, Args&& ... _args) {
    //return sFormat(_buff, N, _format_str, value, std::forward<Args>(_args)...);
    CharBuffSStream ss(_buff);
    __sFormat(ss, _format_str, value, std::forward<Args>(_args)...);
    return ss.stringView();
}


#if 0 // Extension example

struct NewType {
    int a;
    int b;

    friend CharBuffSStream& operator << (CharBuffSStream& _ss, NewType const& _data) {
        _ss << _data.a << "  " << _data.b;
        return _ss;
    }   
};

// Use it as 
NewType instance {1, 2};
char buff[100];
apf::str::sformat(buff, "Result is : {}", instance );
// buff => "Result is : I am:  1, 2"


#endif


} // namespace str
} // namespace apf


#endif // __FEATHERSTATICSSTREAM_H__
