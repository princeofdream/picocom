#ifndef _PHM_SERIALIZER_H_
#define _PHM_SERIALIZER_H_

#include <stdint.h>
#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <cmath>

#include "ara/core/span.h"
#include "ara/core/vector.h"
#include "ara/core/string_view.h"
#include "ara/core/string.h"
#include "ara/core/array.h"
#include "ara/core/result.h"
#include "ara/core/map.h"
#include "UtilityTools.h"

namespace ara
{
    namespace phm
    {

        enum class SEEK_BUFFER : uint8_t
        {
            BUFFER_SET,
            BUFFER_CUR,
            BUFFER_END,
        };

        enum class EndianType : uint8_t
        {
            kBigEndian,
            kLittleEndian,
        };

        template <EndianType>
        struct EndianTrait;

        template <>
        struct EndianTrait<EndianType::kBigEndian>
        {
            template <typename T>
            static T convert(T _value) { return _value; }
        };

        template <>
        struct EndianTrait<EndianType::kLittleEndian>
        {
            template <typename T>
            static T convert(T _value)
            {
                uint8_t *const target_addr = reinterpret_cast<uint8_t *>(&_value);
                std::reverse(target_addr, target_addr + sizeof(T));
                return _value;
            }
        };

        class PhmDeserializer
        {

        public:
            PhmDeserializer(uint8_t const *data, uint32_t size)
            {
                ara::core::Vector<uint8_t> temp(data, data + size);
                m_buff.assign(temp.begin(), temp.end());
                m_current_pos = m_buff.begin();
                m_stream_status = StreamStatus::NORMAL;
            }

            PhmDeserializer(ara::core::Span<uint8_t const> data)
            {
                m_buff.assign(data.begin(), data.end());
                m_current_pos = m_buff.begin();
                m_stream_status = StreamStatus::NORMAL;
            }

            enum class StreamStatus : uint8_t
            {
                NORMAL,
                BUFFER_OVER_FLOW,
                OUT_RANGE,
                DATA_TYPE_NOT_FIT,
                END_ERROR,
                UNKNOW_ERROR
            };

            template <typename T>
            using Result = ara::core::Result<T, StreamStatus>;
            // modi for InputBufferStreamBin
            // private:
        protected:
        public:
            // modi END

            ara::core::Vector<uint8_t> m_buff;
            ara::core::Vector<uint8_t>::iterator m_current_pos;
            StreamStatus m_stream_status;

        public:
            size_t getSize()
            {
                return m_buff.size();
            }

            void getBuffer(uint8_t *data, size_t size)
            {
                for (size_t i = 0; (i < m_buff.size()) && (i < size); i++)
                {
                    data[i] = m_buff[i];
                }
            }

            ara::core::Vector<uint8_t> const &get_buff() const
            {
                return m_buff;
            }

            bool IsStatusOK() const
            {
                return m_stream_status == StreamStatus::NORMAL;
            }

            // get behind m_current_pos all buf
            size_t getCurBuffer(uint8_t *data)
            {
                ara::core::Vector<uint8_t>::iterator iter = m_buff.begin();
                bool copyFlg = false;
                size_t index = 0;
                for (size_t i = 0; i < m_buff.size(); i++, iter++)
                {

                    if (iter == m_current_pos)
                    {
                        copyFlg = true;
                    }
                    if (copyFlg)
                    {
                        data[index] = m_buff[i];
                        index++;
                    }
                }
                return index;
            }

            template <typename T>
            PhmDeserializer &seekg(T size, PhmDeserializer &inputBuffer, SEEK_BUFFER seek_flag)
            {
                if (!inputBuffer.IsStatusOK())
                {

                    return inputBuffer;
                }

                switch (seek_flag)
                {
                case SEEK_BUFFER::BUFFER_CUR:
                    if (((size > 0) && ((uint32_t)(inputBuffer.m_buff.end() - inputBuffer.m_current_pos) < (uint32_t)size)) || ((size < 0) && ((uint32_t)(inputBuffer.m_current_pos - inputBuffer.m_buff.begin()) < (uint32_t)abs(size))))
                    {
                        inputBuffer.m_stream_status = StreamStatus::DATA_TYPE_NOT_FIT;
                        return inputBuffer;
                    }
                    inputBuffer.m_current_pos = inputBuffer.m_current_pos + size;
                    break;
                default:
                    if ((uint32_t)(inputBuffer.m_buff.end() - inputBuffer.m_buff.begin()) < (uint32_t)size)
                    {
                        inputBuffer.m_stream_status = StreamStatus::DATA_TYPE_NOT_FIT;
                        return inputBuffer;
                    }

                case SEEK_BUFFER::BUFFER_SET:
                    if ((size < 0) || ((uint32_t)(inputBuffer.m_buff.end() - inputBuffer.m_buff.begin()) < (uint32_t)size))
                    {
                        inputBuffer.m_stream_status = StreamStatus::DATA_TYPE_NOT_FIT;
                        return inputBuffer;
                    }
                    inputBuffer.m_current_pos = inputBuffer.m_buff.begin() + size;
                    break;

                case SEEK_BUFFER::BUFFER_END:
                    if ((size < 0) || ((uint32_t)(inputBuffer.m_buff.end() - inputBuffer.m_buff.begin()) < (uint32_t)size))
                    {
                        inputBuffer.m_stream_status = StreamStatus::DATA_TYPE_NOT_FIT;
                        return inputBuffer;
                    }
                    inputBuffer.m_current_pos = inputBuffer.m_buff.end() - size;
                    break;
                }

                return inputBuffer;
            }

            // @brief >> operator for integer type like : uint8_t, uint16_t, uint32_t ...
            // template <typename T>
            template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value | std::is_enum<T>::value>::type>
            // template <typename T, typename = typename std::enable_if_t<std::is_integral<T>::value>>
            friend PhmDeserializer &operator>>(PhmDeserializer &inputBuffer, T &_data)
            {
                if (!inputBuffer.IsStatusOK())
                {
                    return inputBuffer;
                }

                if ((uint32_t)(inputBuffer.m_buff.end() - inputBuffer.m_current_pos) < sizeof(T))
                {
                    // printf("decode data err(uint). REMOTECTRLSRV_DECODE_STATUS_BUFFER_OVER_FLOW!!!");
                    inputBuffer.m_stream_status = StreamStatus::DATA_TYPE_NOT_FIT;
                    return inputBuffer;
                }

                uint8_t *const target_addr = reinterpret_cast<uint8_t *>(&_data);
                std::copy(inputBuffer.m_current_pos, inputBuffer.m_current_pos + sizeof(T), target_addr);
                std::reverse(target_addr, target_addr + sizeof(T));
                inputBuffer.m_current_pos += sizeof(T);

                return inputBuffer;
            }

            friend PhmDeserializer &operator>>(PhmDeserializer &inputBuffer, ara::core::String &str)
            {
                if (!inputBuffer.IsStatusOK())
                {
                    return inputBuffer;
                }

                auto str_end_pos = find(inputBuffer.m_current_pos, inputBuffer.m_buff.end(), '\0');
                if (str_end_pos == inputBuffer.m_buff.end())
                {
                    // printf("decode data err(string). REMOTECTRLSRV_DECODE_STATUS_BUFFER_OVER_FLOW!!!");
                    inputBuffer.m_stream_status = StreamStatus::DATA_TYPE_NOT_FIT;
                    return inputBuffer;
                }

                std::copy(inputBuffer.m_current_pos, str_end_pos, std::back_inserter(str));
                inputBuffer.m_current_pos += (str_end_pos - inputBuffer.m_current_pos) + 1;

                return inputBuffer;
            }

            /* Data type : ara::core::u16String (String that supports UTF-16)
             * Default   : Big endian
             * Function  : The inputBuffer contains the BOM.
                           Confirm the big endian and little endian of String according to BOM, and deserialize according to uint16_t type.
             */
            friend PhmDeserializer &operator>>(PhmDeserializer &inputBuffer, ara::core::u16String &str)
            {
                // Find the end of a UTF-16 string (two consecutive 0).
                auto findStrEnd = [&](const uint8_t &i, const uint8_t &j)
                {
                    return ((i == 0) && (j == 0));
                };
                auto str_end_pos = adjacent_find(inputBuffer.m_current_pos, inputBuffer.m_buff.end(), findStrEnd);
                if (str_end_pos == inputBuffer.m_buff.end())
                {
                    inputBuffer.m_stream_status = StreamStatus::DATA_TYPE_NOT_FIT;
                    return inputBuffer;
                }

                // Get the length of the UTF-16 string (even). If the length is odd, add 1 to the length.
                size_t distance_val = distance(inputBuffer.m_current_pos, str_end_pos);
                if (distance_val % 2 == 1)
                {
                    str_end_pos += 1;
                    distance_val += 1;
                }

                // Get BOM from inputBuffer. If there is no BOM, the default is big endian for deserialization.
                uint16_t bom = 0xFEFF;
                if (inputBuffer.m_current_pos - inputBuffer.m_buff.begin() > 1)
                {
                    uint8_t *const target_addr = reinterpret_cast<uint8_t *>(&bom);
                    std::copy(inputBuffer.m_current_pos - 2, inputBuffer.m_current_pos, target_addr);
                    std::reverse(target_addr, target_addr + sizeof(uint16_t));
                }
                for (size_t i = 0; i < distance_val / 2; i++)
                {
                    uint16_t value = 0;
                    inputBuffer >> value;
                    (bom == 0xFFFE) ? value = EndianTrait<EndianType::kLittleEndian>::convert(value) : value = EndianTrait<EndianType::kBigEndian>::convert(value);
                    str.push_back(value);
                }
                inputBuffer.m_current_pos += 2;

                return inputBuffer;
            }

            template <typename T, size_t N>
            friend PhmDeserializer &operator>>(PhmDeserializer &inputBuffer, std::array<T, N> &ary)
            {
                if (!inputBuffer.IsStatusOK())
                {
                    return inputBuffer;
                }

                if ((size_t)(inputBuffer.m_buff.end() - inputBuffer.m_current_pos) < N * sizeof(T))
                {
                    // printf("decode data err(array). REMOTECTRLSRV_DECODE_STATUS_BUFFER_OVER_FLOW!!!");
                    inputBuffer.m_stream_status = StreamStatus::DATA_TYPE_NOT_FIT;
                    return inputBuffer;
                }

                for (size_t i = 0; i < N; i++)
                {
                    inputBuffer >> ary[i];
                }

                return inputBuffer;
            }

            template <typename T>
            Result<T> tryRead()
            {
                auto _current_pos = m_current_pos;
                auto res = read<T>();
                m_current_pos = _current_pos;
                m_stream_status = StreamStatus::NORMAL;
                return res;
            }

            template <typename T>
            Result<T> read()
            {
                T data;
                *this >> data;
                if (!IsStatusOK())
                {
                    return Result<T>::FromError(m_stream_status);
                }
                return data;
            }

            template <typename T>
            friend PhmDeserializer &operator>>(PhmDeserializer &_deserilizer, std::vector<T> &_data)
            {
                size_t len = 0;
                _deserilizer >> len;

                T temp = 0;
                for (size_t i = 0; i < len; i++)
                {
                    _deserilizer >> temp;
                    _data.push_back(temp);
                }
                // _data.insert(_data.end(), _deserilizer.m_current_pos, _deserilizer.m_current_pos + len*sizeof(T));
                /* auto res = _deserilizer.read<T>();
                if (res.HasValue())
                {
                    _data.push_back(res.Value());
                }*/
                return _deserilizer;
            }

#if 0
    friend PhmDeserializer& operator >> (PhmDeserializer& inputBuffer, ara::core::Vector<uint8_t>& vec) {

        size_t len = inputBuffer.m_buff.end() - inputBuffer.m_current_pos;

        std::copy(inputBuffer.m_current_pos, inputBuffer.m_buff.end(), vec.begin());

        inputBuffer.m_current_pos += len;

        return inputBuffer;
    }

#endif
        };

        class PhmDeserializerBin : public PhmDeserializer
        {
            union UINT16TYPE
            {
                uint8_t ch[2];
                uint16_t idata;
            };
            union UINT32TYPE
            {
                uint8_t ch[4];
                uint16_t idata;
            };

        public:
            PhmDeserializerBin(uint8_t *data, uint32_t size) : PhmDeserializer(data, size)
            {
            }

            PhmDeserializerBin(ara::core::Span<uint8_t const> data) : PhmDeserializer(data)
            {
            }

            friend PhmDeserializer &operator>>(PhmDeserializerBin &inputBuffer, uint16_t &idata)
            {
                if (!inputBuffer.IsStatusOK())
                {
                    return inputBuffer;
                }

                // std::copy(inputBuffer.m_current_pos, inputBuffer.m_current_pos+2, idata);
                union UINT16TYPE u16typ;
                u16typ.ch[0] = *inputBuffer.m_current_pos;
                u16typ.ch[1] = *(inputBuffer.m_current_pos + 1);
                idata = u16typ.idata;
                inputBuffer.m_current_pos += 2;
                // printf("-----PhmDeserializerBin uint16_t-----\r\n");
                return inputBuffer;
            }
        };

        const uint32_t CONTANT_SIZE = 1000u;
        class PhmSerializer
        {
        public:
            enum class StreamStatus : uint8_t
            {
                NORMAL,
                BUFFER_OVER_FLOW,
                OUT_RANGE,
                DATA_TYPE_NOT_FIT,
                END_ERROR,
                UNKNOW_ERROR
            };
            bool isStatusOK()
            {
                return m_stream_status == StreamStatus::NORMAL;
            }

            PhmSerializer()
            {
                m_stream_status = StreamStatus::NORMAL;
            }

            template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value | std::is_enum<T>::value>::type>
            // template <typename T, typename = typename std::enable_if_t<std::is_integral<T>::value>>
            friend PhmSerializer &operator<<(PhmSerializer &outputBuffer, T _data)
            {
                if (!outputBuffer.isStatusOK())
                {
                    // printf("encode data err(uint). REMOTECTRLSRV_DECODE_STATUS_BUFFER_OVER_FLOW!!!\n");
                    return outputBuffer;
                }
                uint8_t *target_addr = reinterpret_cast<uint8_t *>(&_data);
                for (int i = sizeof(T) - 1; i >= 0; i--)
                {
                    outputBuffer.m_buffer.push_back(target_addr[i]);
                }

                return outputBuffer;
            }

            friend PhmSerializer &operator<<(PhmSerializer &outputBuffer, ara::core::String str)
            {
                if (!outputBuffer.isStatusOK())
                {
                    // printf("encode data err(string). REMOTECTRLSRV_DECODE_STATUS_BUFFER_OVER_FLOW!!!\n");
                    return outputBuffer;
                }
                for (size_t i = 0; i < str.length(); i++)
                {
                    outputBuffer.m_buffer.push_back(str[i]);
                }
                // String ends with "\0"
                outputBuffer.m_buffer.push_back('\0');

                return outputBuffer;
            }

            /* Data type : ara::core::u16String (String that supports UTF-16)
             * Default   : Big endian
             * Function  : The outputBuffer contains the BOM.
                           Confirm the big endian and little endian of String according to BOM, and serialize according to uint16_t type.
             */
            friend PhmSerializer &operator<<(PhmSerializer &outputBuffer, ara::core::u16String str)
            {
                uint16_t bom = 0xFEFF;

                // Get BOM from outputBuffer
                if (outputBuffer.getSize() > 1)
                {
                    uint8_t *const target_addr = reinterpret_cast<uint8_t *>(&bom);
                    std::copy(outputBuffer.m_buffer.end() - 2, outputBuffer.m_buffer.end(), target_addr);
                    std::reverse(target_addr, target_addr + sizeof(uint16_t));
                }
                // Confirm big endian and little endian according to BOM.
                // The BOM has been serialized in the function FixLenString or DynLenString.
                for (size_t i = 0; i < str.length(); i++)
                {
                    (bom == 0xFFFE) ? outputBuffer << EndianTrait<EndianType::kLittleEndian>::convert(str[i]) : outputBuffer << EndianTrait<EndianType::kBigEndian>::convert(str[i]);
                }
                // String ends with "\0"
                outputBuffer.m_buffer.insert(outputBuffer.m_buffer.end(), 2, '\0');

                return outputBuffer;
            }

            friend PhmSerializer &operator<<(PhmSerializer &outputBuffer, ara::core::Span<const uint8_t> ary)
            {
                if (!outputBuffer.isStatusOK())
                {
                    // printf("encode data err(array). REMOTECTRLSRV_DECODE_STATUS_BUFFER_OVER_FLOW!!!\n");
                    return outputBuffer;
                }

                for (size_t i = 0; i < static_cast<size_t>(ary.size()); i++)
                {
                    outputBuffer.m_buffer.push_back(ary[i]);
                }

                return outputBuffer;
            }

            template <typename T>
            friend PhmSerializer &operator<<(PhmSerializer &outputBuffer, ara::core::Vector<T> vec)
            {
                if (!outputBuffer.isStatusOK())
                {
                    // printf("encode data err(array). REMOTECTRLSRV_DECODE_STATUS_BUFFER_OVER_FLOW!!!\n");
                    return outputBuffer;
                }

                outputBuffer << static_cast<size_t>(vec.size());

                for (size_t i = 0; i < static_cast<size_t>(vec.size()); i++)
                {
                    outputBuffer << vec[i];
                }

                return outputBuffer;
            }

            template <typename T, size_t N>
            friend PhmSerializer &operator<<(PhmSerializer &outputBuffer, std::array<T, N> ary)
            {
                if (!outputBuffer.isStatusOK())
                {
                    // printf("encode data err(array). REMOTECTRLSRV_DECODE_STATUS_BUFFER_OVER_FLOW!!!\n");
                    return outputBuffer;
                }
                for (size_t i = 0; i < ary.size(); i++)
                {
                    outputBuffer << ary[i];
                }

                return outputBuffer;
            }

            /**add for out << out2 << out3 **/
            friend PhmSerializer &operator<<(PhmSerializer &outputBuffer, PhmSerializer &out2)
            {
                if (!outputBuffer.isStatusOK())
                {
                    // printf("encode data err(string). REMOTECTRLSRV_DECODE_STATUS_BUFFER_OVER_FLOW!!!\n");
                    return outputBuffer;
                }
                size_t size2 = out2.getSize();
                size_t size1 = outputBuffer.getSize();
                if (size1 + size2 > CONTANT_SIZE)
                {
                    outputBuffer.m_stream_status = StreamStatus::BUFFER_OVER_FLOW;
                    return outputBuffer;
                }
                uint8_t arr[CONTANT_SIZE] = {0};
                out2.getBuffer(arr, size2);
                for (size_t i = 0; i < size2; i++)
                {
                    outputBuffer.m_buffer.push_back(arr[i]);
                }

                return outputBuffer;
            }
            /**add end**/
#if 0
    friend PhmSerializer& operator << (PhmSerializer& outputBuffer, ara::core::Vector<uint8_t>& vec) {
        for (auto iter = vec.begin(); iter != vec.end(); ++iter)
        {
            outputBuffer.m_buffer.push_back(*iter);
        }

        return outputBuffer;
    }
#endif
            size_t getSize()
            {
                return m_buffer.size();
            }

            void getBuffer(uint8_t *data, size_t size)
            {
                for (size_t i = 0; (i < m_buffer.size()) && (i < size); i++)
                {
                    data[i] = m_buffer[i];
                }
            }

            ara::core::Span<const uint8_t> getBuffSnapShot()
            {
                return m_buffer;
            }

        public:
            ara::core::Vector<uint8_t> m_buffer;
            // ara::core::Vector<uint8_t>::iterator m_current_position;
            StreamStatus m_stream_status;
        };

        template <typename T, EndianType E = EndianType::kBigEndian, typename = typename std::enable_if<std::is_arithmetic<T>::value | std::is_enum<T>::value>::type>
        class BinInt
        {
        public:
            friend PhmSerializer &operator<<(PhmSerializer &output_buffer, BinInt<T, E> const &_data)
            {
                output_buffer << EndianTrait<E>::convert(_data.m_value);
                return output_buffer;
            }

            friend PhmDeserializer &operator>>(PhmDeserializer &input_buffer, BinInt<T, E> &_data)
            {
                input_buffer >> _data.m_value;
                _data.m_value = EndianTrait<E>::convert(_data.m_value);

                return input_buffer;
            }

            operator T &()
            {
                return m_value;
            }

            operator T const &() const
            {
                return m_value;
            }

            BinInt<T, E> &operator=(T _value)
            {
                m_value = _value;
                return *this;
            }

        private:
            T m_value;
        };

        /*L : type of lengthfield, uint8_t/uint16_t/uint32_t*/
        template <class _Kty, class _Ty, typename L = uint32_t, class _Pr = std::less<_Kty>, class _Alloc = std::allocator<std::pair<const _Kty, _Ty>>>
        class Map : public ara::core::Map<_Kty, _Ty, _Pr, _Alloc>
        {
        public:
            L getMapLength() const { return m_length; }

            friend PhmDeserializer &operator>>(PhmDeserializer &_input_buffer, Map &_data)
            {
                _input_buffer >> _data.m_length;
                auto start_pos = _input_buffer.m_current_pos;
                _data.clear();
                L deserializedsize = 0;
                while (_data.m_length > deserializedsize)
                {
                    _Kty tempkey;
                    _Ty tempvalue;
                    _input_buffer >> tempkey;
                    _input_buffer >> tempvalue;
                    _data.insert(std::pair<_Kty, _Ty>(tempkey, tempvalue));
                    deserializedsize = _input_buffer.m_current_pos - start_pos;
                }
                return _input_buffer;
            }

            friend PhmSerializer &operator<<(PhmSerializer &output_buffer, Map &_data)
            {
                size_t lengthoffset = output_buffer.getSize();
                output_buffer << _data.m_length;

                for (auto &item : _data)
                {
                    output_buffer << item.first;
                    output_buffer << item.second;
                }

                _data.m_length = static_cast<L>(output_buffer.getSize() - lengthoffset - sizeof(L));
                uint8_t *target_addr = reinterpret_cast<uint8_t *>(&_data.m_length);
                for (size_t i = 0; i < sizeof(L); i++)
                {
                    output_buffer.m_buffer[lengthoffset + (sizeof(L) - 1 - i)] = target_addr[i];
                }

                return output_buffer;
            }

        private:
            L m_length = 0;
        };

    } // namespace apf
}

#endif
