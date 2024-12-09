#ifndef __FEATHERSTATICSSTREAM_H__
#define __FEATHERSTATICSSTREAM_H__

#include <string>

#include "ara/core/string_view.h"


namespace apf {
namespace str {

class CharBuffSStream {
public:

    template<size_t N>
    CharBuffSStream(char (&_buff)[N])
        : m_buff(_buff)
        , m_buff_size(N)
        , m_formated_size(0)
    {
        std::fill(_buff, _buff + m_buff_size, 0);
    }

    CharBuffSStream& operator << (ara::core::StringView _message) {

        formatStrToMsgBuff(_message);

        return *this;
    }

    CharBuffSStream& operator << (char _a) {

        formatStrToMsgBuff(_a);

        return *this;
    }

    CharBuffSStream& operator << (float _data) {

        formatStrToMsgBuff("%f", _data);

        return *this;
    }

    CharBuffSStream& operator << (double _data) {

        formatStrToMsgBuff("%lf", _data);

        return *this;
    }

    template <typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
    CharBuffSStream& operator << (T _data) {
        if (sizeof(T) > sizeof(uint32_t))
        {
            if (std::is_signed<T>::value)
            {
                formatStrToMsgBuff("%lld", _data);
            }
            else
            {
                formatStrToMsgBuff("%llu", _data);
            }
        }
        else
        {
            if (std::is_signed<T>::value)
            {
                formatStrToMsgBuff("%d", _data);
            }
            else
            {
                formatStrToMsgBuff("%u", _data);
            }
        }

        return *this;
    }

    ara::core::StringView stringView() const {
        return { m_buff, m_formated_size };
    }

    size_t getFormatedSize() const {
        return m_formated_size;
    }

private:

    void formatStrToMsgBuff(ara::core::StringView _data) {
        if (m_formated_size >= m_buff_size) {
            return;
        }
        auto end_pos = (std::min)(_data.size(), (m_buff_size - m_formated_size - 1));
        std::copy(_data.begin(), _data.begin() + end_pos, m_buff + m_formated_size);
        m_formated_size += end_pos;
        m_buff[m_formated_size] = '\0';
    }

    void formatStrToMsgBuff(char _a) {
        if (m_formated_size >= m_buff_size) {
            return;
        }
        m_buff[m_formated_size] = _a;
        m_formated_size += 1;
        m_buff[m_formated_size] = '\0';
    }

    template <typename T>
    void formatStrToMsgBuff(char const* _format, T const& _data) {
        size_t size_need = 1 + snprintf(nullptr, 0, _format, _data);
        if (size_need >= (m_buff_size - m_formated_size)) {
            return;
        }
        auto size = snprintf(m_buff + m_formated_size, m_buff_size - m_formated_size, _format, _data);
        m_formated_size += size;
    }

private:
    char * const m_buff;
    size_t const m_buff_size;
    size_t       m_formated_size;
};


template <size_t N = 512>
class StaticSStream : public CharBuffSStream {
public:
    StaticSStream() : CharBuffSStream(m_array) {
    }

private:
    char     m_array[N];
};

#if 0 // Extension example

struct NewType {
    int a;
    int b;

    friend CharBuffSStream& operator << (CharBuffSStream& _ss, NewType const& _data) {
        _ss << _data.a << "  " << _data.b;
        return _ss;
    }   
};


#endif

} // namespace str
} // namespace apf


#endif // __FEATHERSTATICSSTREAM_H__
