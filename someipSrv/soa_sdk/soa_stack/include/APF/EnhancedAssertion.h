#ifndef __FEATHERENSURE_H__
#define __FEATHERENSURE_H__

#include <cstdio>
#include <string>
#include <tuple>
#include <sstream>
#include <functional>
#include <iostream>

#include "StaticSStream.h"
#include "StaticBuffFormat.h"
#include "ara/core/string_view.h"


using namespace std::literals;

namespace apf {
namespace dbg {


class FeatherAssert {
public:
    enum Level {
        INFO = 0,
        DEBUG,
        WARNING,
        ERR
    };
public:

    FeatherAssert(ara::core::StringView _expr, ara::core::StringView _prefix_info) : FEATHER_ASSERT_A(*this), FEATHER_ASSERT_B(*this) {
        if (_expr == "" && _prefix_info == "") {
            return;
        }
        str::StaticSStream<512> _buffer;
        apf::str::sFormat(_buffer, "{} {}"sv, _prefix_info, _expr);
        m_log_out(_buffer.stringView());
    }

    template <typename ...Args>
    FeatherAssert& Msg(ara::core::StringView _format, Args&& ... _args) {
        str::StaticSStream<512> _buffer;
        apf::str::sFormat(_buffer, _format, std::forward<Args>(_args)...);
        m_log_out(_buffer.stringView());
        return *this;
    }

    FeatherAssert& printf_context(ara::core::StringView _file_name, uint32_t _line_num) {

        Msg("File : {}"sv, _file_name);
        Msg("Line : {}"sv, _line_num);
        Msg("With context of : ");

        return *this;
    }

    FeatherAssert& FEATHER_ASSERT_A;
    FeatherAssert& FEATHER_ASSERT_B;

    template <typename T>
    FeatherAssert& print_val(T const& _var, ara::core::StringView _name) {
        Msg("\t {} : {}"sv, _name, _var);
        return *this;
    }

private:
    static std::function<void(ara::core::StringView _msg)> m_log_out;
};

}
}


//uint8_t FeatherAssert::m_level = DEBUG;

#define MAKE_ASSERT_LINENAME_CAT(name, line) name##line
#define MAKE_ASSERT_LINENAME(name, line)     MAKE_ASSERT_LINENAME_CAT(name, line)
#define MAKE_ASSERT(expr)                    MAKE_ASSERT_LINENAME(FeatherAssert, __LINE__)(expr)

#define FEATHER_ASSERT_A(x) FEATHER_ASSERT_OP(x,B)
#define FEATHER_ASSERT_B(x) FEATHER_ASSERT_OP(x,A)
#define FEATHER_ASSERT_OP(x, next)  FEATHER_ASSERT_A.print_val((x), #x).FEATHER_ASSERT_##next

#define ENSURE(expr)     if((expr)); else throw apf::dbg::FeatherAssert(#expr, "!!!MUST NOT HAPPEN ERROR : ").printf_context(__FILE__, __LINE__).FEATHER_ASSERT_A 
#define EXPECT(expr)     if((expr)); else apf::dbg::FeatherAssert(#expr, "Warning ! Expectation Not Satisfied, Check it :").printf_context(__FILE__, __LINE__).FEATHER_ASSERT_A 

#define FEATHER_LOG(log_msg, ...)     apf::dbg::FeatherAssert("", "").Msg(log_msg, ##__VA_ARGS__)
#define FEATHER_LOG_ARG(log_msg, ...) apf::dbg::FeatherAssert("", "").Msg(log_msg, ##__VA_ARGS__).FEATHER_ASSERT_A

#if 0 // Sample 

#include "EnhancedAssertion.h"

void test_function(int a, int b) {
    ENSURE(a > b && a != 0)(a)(b).Msg("Some Problem");
    EXPECT(a > b && a != 0)(a)(b).Msg("Some Problem");
}


#endif

#endif // __FEATHERENSURE_H__
