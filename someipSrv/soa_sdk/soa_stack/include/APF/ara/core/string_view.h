#ifndef __STRING_VIEW_H__
#define __STRING_VIEW_H__

//#include <string_view>
#include <vector>
#include <stdint.h>

#include "../../nonstd/string_view.hpp"

// #include "../EnhancedAssertion.h"


namespace ara {
namespace core {

using StringView = nonstd::string_view;
using StringViewu16 = nonstd::u16string_view;


// class StringUility {
// public:
//     static std::string& Ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
//     {
//         str.erase(0, str.find_first_not_of(chars));
//         return str;
//     }

//     static std::string& Rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
//     {
//         str.erase(str.find_last_not_of(chars) + 1);
//         return str;
//     }

//     static std::string& Trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
//     {
//         return Ltrim(Rtrim(str, chars), chars);
//     }

//     static std::vector<std::string> Split(const std::string& s, char delimiter) {
//         std::vector<std::string> tokens;
//         std::string token;
//         std::istringstream tokenStream(s);
//         while (std::getline(tokenStream, token, delimiter)) {
//             tokens.push_back(token);
//         }
//         return tokens;
//     }
// };

namespace temp_detail {
     
    // We have this in UtiltiyTools.h whitch also includes string_view.h
    // For avoid loop include, define another one here.
    template <typename U, typename T>
    U safeIntCast(T _value) {
        ENSURE(_value < static_cast<T>((std::numeric_limits<U>::max)()))(_value);

        return static_cast<U>(_value);
    }
}

class StringViewUility {
public:
    static StringView& Ltrim(StringView& str, StringView chars = "\t\n\v\f\r ")
    {
        auto pos = str.find_first_not_of(chars);
        auto remove_count = pos == StringView::npos ? str.size() : pos;
        str.remove_prefix(remove_count);
        return str;
    }

    static StringView& Rtrim(StringView& str, StringView chars = "\t\n\v\f\r ")
    {
        auto pos = str.find_last_not_of(chars);
        auto remove_count = pos == StringView::npos ? str.size() : str.size() - pos - 1;
        str.remove_suffix(remove_count);
        return str;
    }

    static StringView& Trim(StringView& str, StringView chars = "\t\n\v\f\r ")
    {
        return Ltrim(Rtrim(str, chars), chars);
    }

    // using std::from_chars if C++17
    //    to avoid std::string
    // @todo Even: support other system : binary hexadecimal and Octal
    template <typename T = uint32_t>
    static T ToInt(StringView _str, uint8_t system = 10) {
        StringViewUility::Trim(_str, " \t");
        int pos = 1;
        int rslt = 0;
        for (auto iter = _str.crbegin(); iter != _str.crend(); ++iter) {
            // ENSURE(std::isdigit(*iter))(_str);
            rslt += ((*iter - '0') * pos);
            pos *= 10;
        }
        return temp_detail::safeIntCast<T>(rslt);
    }

    static std::vector<StringView> Split(StringView _sv, StringView delimiter) {
        std::vector<StringView> rslt;
        size_t pos = 0;
        while ((pos = _sv.find(delimiter)) != std::string::npos) {
            auto token = _sv.substr(0, pos);
            rslt.push_back(token);
            _sv.remove_prefix(pos + 1);
        }
        if (_sv.length() > 0) {
            rslt.push_back(_sv);
        }
        return rslt;
    }
};

template <size_t N>
size_t CopyToCharArray(char(&_ary)[N], StringView _str) {
    memset(_ary, 0x00U, N);
    auto end_pos = (std::min)(_str.size(), N - 1);
    std::copy(_str.begin(), _str.begin() + end_pos, std::begin(_ary));

    return end_pos;
}


}
}


#endif // __STRING_VIEW_H__
