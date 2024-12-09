#ifndef _ARA_COM_OUTPUT_MESSAGE_STREAM_HPP_
#define _ARA_COM_OUTPUT_MESSAGE_STREAM_HPP_

#include <cassert>
#include <cstring>
#include <iomanip>
#include <limits>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "CommonAPI/Deployment.hpp"
#include "CommonAPI/OutputStream.hpp"
#include "StringEncoder.hpp"
#include <bitset>

using namespace CommonAPI;

namespace ara {
namespace com {

/**
 * Used to mark the position of a pointer within an array of bytes.
 */
typedef uint32_t position_t;

/**
 * @class OutputMessageStream
 *
 * Used to serialize and write data into a #Message. For all data types that may be written to a #Message, a "<<"-operator should be defined to handle the writing
 * (this operator is predefined for all basic data types and for vectors). The signature that has to be written to the #Message separately is assumed
 * to match the actual data that is inserted via the #OutputMessageStream.
 */
class OutputStream: public CommonAPI::OutputStream<OutputStream> {
public:

    OutputStream() : errorOccurred_(false) {
    }

    /**
     * Destructor; does not call the destructor of the referred #Message. Make sure to maintain a reference to the
     * #Message outside of the stream if you intend to make further use of the message, e.g. in order to send it,
     * now that you have written some payload into it.
     */
    ~OutputStream() {
    }

    OutputStream &writeValue(const bool &_value, const EmptyDeployment *_depl) {
        return _writeValue(_value);
    }

    OutputStream &writeValue(const int8_t &_value, const EmptyDeployment *_depl) {
        return _writeValue(_value);
    }
    OutputStream &writeValue(const int16_t &_value, const EmptyDeployment *_depl) {
        return _writeValue(_value);
    }
    OutputStream &writeValue(const int32_t &_value, const EmptyDeployment *_depl) {
        return _writeValue(_value);
    }
    OutputStream &writeValue(const int64_t &_value, const EmptyDeployment *_depl) {
        return _writeValue(_value);
    }

    OutputStream &writeValue(const uint8_t &_value, const EmptyDeployment *_depl) {
        return _writeValue(_value);
    }
    OutputStream &writeValue(const uint16_t &_value, const EmptyDeployment *_depl) {
        return _writeValue(_value);
    }
    OutputStream &writeValue(const uint32_t &_value, const EmptyDeployment *_depl) {
        return _writeValue(_value);
    }
    OutputStream &writeValue(const uint64_t &_value, const EmptyDeployment *_depl) {
        return _writeValue(_value);
    }

    OutputStream &writeValue(const float &_value, const EmptyDeployment *_depl) {
        return _writeValue(_value);
    }
    OutputStream &writeValue(const double &_value, const EmptyDeployment *_depl) {
        return _writeValue(_value);
    }

    OutputStream &writeValue(const std::string &_value, const EmptyDeployment *_depl) {
        return writeValue(_value, static_cast<const StringDeployment*>(nullptr));
    }
    OutputStream &writeValue(const std::string &_value, const StringDeployment *_depl) {

        bool errorOccurred = false;
        size_t size, terminationSize(2);
        size_t bomSize(2);
        byte_t* bytes;

        //Determine string length
        if (_depl != nullptr)
        {
            EncodingStatus status = EncodingStatus::SUCCESS;
            std::shared_ptr<StringEncoder> encoder = std::make_shared<StringEncoder>();

            switch (_depl->stringEncoding_)
            {
            case StringEncoding::UTF16BE:
                encoder->utf8To16((byte_t*)_value.c_str(), BIG_ENDIAN, status, &bytes, size);
                break;

            case StringEncoding::UTF16LE:
                encoder->utf8To16((byte_t*)_value.c_str(), LITTLE_ENDIAN, status, &bytes, size);
                break;

            default:
                bytes = (byte_t*)(_value.c_str());
                size = _value.size();
                bomSize = 3;
                terminationSize = 1;
                break;
            }

            if (status != EncodingStatus::SUCCESS)
            {
                //TODO error handling
            }

        }
        else
        {
            bytes = (byte_t*)(_value.c_str());
            size = _value.size();
            bomSize = 3;
            terminationSize = 1;
        }

        //write string length
        if (_depl != nullptr) {
            if (_depl->stringLengthWidth_ == 0
                && _depl->stringLength_ != size + terminationSize + bomSize) {
                errorOccurred = true;
            }
            else {
                _writeValue(uint32_t(size + terminationSize + bomSize),
                    _depl->stringLengthWidth_);
            }
        }
        else {
            _writeValue(uint32_t(size + terminationSize + bomSize), 4);
        }


        if (!errorOccurred) {
            // Write BOM
            _writeBom(_depl);

            // Write sting content
            _writeRaw(bytes, size);

            // Write termination
            const byte_t termination[] = { 0x00, 0x00 };
            _writeRaw(termination, terminationSize);
        }

        if (bytes != (byte_t*)_value.c_str()) {
            delete[] bytes;
        }

        return (*this);
    }

    OutputStream &writeValue(const ByteBuffer &_value, const ByteBufferDeployment *_depl) {
        uint32_t byteBufferMinLength = (_depl ? _depl->byteBufferMinLength_ : 0);
        uint32_t byteBufferMaxLength = (_depl ? _depl->byteBufferMaxLength_ : 0xFFFFFFFF);

        pushPosition();     // Start of length field
        _writeValue(0, 4);  // Length field placeholder
        pushPosition();     // Start of vector data

        if (byteBufferMinLength != 0 && _value.size() < byteBufferMinLength) {
            errorOccurred_ = true;
        }
        if (byteBufferMaxLength != 0 && _value.size() > byteBufferMaxLength) {
            errorOccurred_ = true;
        }

        if (!hasError()) {
            // Write array/vector content
            for (auto i : _value) {
                writeValue(i, nullptr);
                if (hasError()) {
                    break;
                }
            }
        }

        // Write actual value of length field
        size_t length = getPosition() - popPosition();
        size_t position2Write = popPosition();
        _writeValueAt(uint32_t(length), 4, uint32_t(position2Write));

        return (*this);
    }

    OutputStream &writeValue(const Version &_value, const EmptyDeployment *_depl) {
        _writeValue(_value.Major);
        _writeValue(_value.Minor);
        return (*this);
    }

    OutputStream &_writeValue(const uint32_t &_value, const uint8_t &_width) {
        switch (_width) {
        case 1:
            if (_value > std::numeric_limits<uint8_t>::max()) {
                errorOccurred_ = true;
            }
            _writeValue(static_cast<uint8_t>(_value));
            break;
        case 2:
            if (_value > std::numeric_limits<uint16_t>::max()) {
                errorOccurred_ = true;
            }
            _writeValue(static_cast<uint16_t>(_value));
            break;
        case 4:
            if (_value > std::numeric_limits<uint32_t>::max()) {
                errorOccurred_ = true;
            }
            _writeValue(static_cast<uint32_t>(_value));
            break;
        }

        return (*this);
    }
    OutputStream &_writeValueAt(const uint32_t &_value, const uint8_t &_width, const uint32_t &_position) {
        switch (_width) {
        case 1:
            if (_value > std::numeric_limits<uint8_t>::max()) {
                errorOccurred_ = true;
            }
            _writeValueAt(static_cast<uint8_t>(_value), _position);
            break;
        case 2:
            if (_value > std::numeric_limits<uint16_t>::max()) {
                errorOccurred_ = true;
            }
            _writeValueAt(static_cast<uint16_t>(_value), _position);
            break;
        case 4:
            if (_value > std::numeric_limits<uint32_t>::max()) {
                errorOccurred_ = true;
            }
            _writeValueAt(static_cast<uint32_t>(_value), _position);
            break;
        }

        return (*this);
    }


     template<typename Base_>
     OutputStream &writeValue(const Enumeration<Base_> &_value, const EmptyDeployment *) {
         writeValue(static_cast<Base_>(_value), nullptr);
         return (*this);
     }

     template<class Deployment_, typename Base_>
     OutputStream &writeValue(const Enumeration<Base_> &_value, const Deployment_ *_depl) {
         if (_depl != nullptr) {
             switch (_depl->width_) {
             case 1:
             {
                 uint8_t tmpValue1 = static_cast<uint8_t>(_value);
                 writeValue(tmpValue1, nullptr);
             }
             break;

             case 2:
             {
                 uint16_t tmpValue2 = static_cast<uint16_t>(_value);
                 writeValue(tmpValue2, nullptr);
             }
             break;

             default:
                 writeValue(static_cast<Base_>(_value), nullptr);
                 break;
             }
         } else {
             writeValue(static_cast<Base_>(_value), nullptr);
         }
         return (*this);
    }

    template<typename... Types_>
    OutputStream &writeValue(const Struct<Types_...> &_value,
                             const EmptyDeployment *_depl) {
        // don't write length field as default length width is 0
        if(!hasError()) {
            // Write struct content
            const auto itsSize(std::tuple_size<std::tuple<Types_...>>::value);
            StructWriter<itsSize-1, OutputStream, Struct<Types_...>, EmptyDeployment>{}((*this), _value, _depl);
        }
        return (*this);
    }

    template<typename Deployment_, typename... Types_>
    OutputStream &writeValue(const Struct<Types_...> &_value,
                             const Deployment_ *_depl) {
        uint8_t structLengthWidth = (_depl ? _depl->structLengthWidth_ : 0);

        if (structLengthWidth != 0) {
            pushPosition();
            // Length field placeholder
            _writeValue(0, structLengthWidth);
            pushPosition(); // Start of struct data
        }

        if(!hasError()) {
            // Write struct content
            const auto itsSize(std::tuple_size<std::tuple<Types_...>>::value);
            StructWriter<itsSize-1, OutputStream, Struct<Types_...>, Deployment_>{}((*this), _value, _depl);
        }

        // Write actual value of length field
        if (structLengthWidth != 0) {
            size_t length = getPosition() - popPosition();
            size_t position = popPosition();
            _writeValueAt(uint32_t(length), structLengthWidth, uint32_t(position));
        }

        return (*this);
    }

    template<class PolymorphicStruct_>
    OutputStream &writeValue(const std::shared_ptr<PolymorphicStruct_> &_value,
                             const EmptyDeployment *_depl = nullptr) {
        if (_value) {
            _writeValue(_value->getSerial());
            if (!hasError()) {
                _value->template writeValue<OutputStream>((*this), _depl);
            }
        }
        return (*this);
    }

    template<class PolymorphicStruct_, typename Deployment_>
    OutputStream &writeValue(const std::shared_ptr<PolymorphicStruct_> &_value,
                             const Deployment_ *_depl = nullptr) {
        if (_value) {
            _writeValue(_value->getSerial());
            if (!hasError()) {
                _value->template writeValue<OutputStream>((*this), _depl);
            }
        }
        return (*this);
    }
    template<typename Deployment_, typename... Types_>
    OutputStream &writeValue(const Variant<Types_...> &_value,
                             const Deployment_ *_depl) {
        bool unionDefaultOrder = (_depl ? _depl->unionDefaultOrder_ : true);
        uint8_t unionLengthWidth = (_depl ? _depl->unionLengthWidth_ : 4);
        uint8_t unionTypeWidth = (_depl ? _depl->unionTypeWidth_ : 4);

        if (unionDefaultOrder) {
            pushPosition();
            _writeValue(0, unionLengthWidth);
            _writeValue(uint8_t(_value.getMaxValueType()) - _value.getValueType() + 1, unionTypeWidth);
            pushPosition();
        } else {
            _writeValue(uint8_t(_value.getMaxValueType()) - _value.getValueType() + 1, unionTypeWidth);
            pushPosition();
            _writeValue(0, unionLengthWidth);
            pushPosition();
        }

        if (!hasError()) {
            OutputStreamWriteVisitor<OutputStream> valueVisitor(*this);
            ApplyStreamVisitor<OutputStreamWriteVisitor<OutputStream>,
                 Variant<Types_...>, Deployment_, Types_...>::visit(valueVisitor, _value, _depl);
        }

        size_t length = getPosition() - popPosition();
        size_t position = popPosition();

        // Write actual value of length field
        if (unionLengthWidth != 0) {
            _writeValueAt(uint32_t(length), unionLengthWidth, uint32_t(position));
        } else {
            size_t paddingCount = _depl->unionMaxLength_ - length;
            if (paddingCount < 0) {
                errorOccurred_ = true;
            } else {
                for(size_t i = 0; i < paddingCount; i++) {
                    _writeValue('\0');
                }
            }
        }

        return (*this);
    }

    template<typename ElementType_, typename ElementDepl_>
    OutputStream &writeValue(const std::vector<ElementType_> &_value,
                             const ArrayDeployment<ElementDepl_> *_depl) {
        uint8_t arrayLengthWidth = (_depl ? _depl->arrayLengthWidth_ : 4);
        uint32_t arrayMinLength = (_depl ? _depl->arrayMinLength_ : 0);
        uint32_t arrayMaxLength = (_depl ? _depl->arrayMaxLength_ : 0xFFFFFFFF);

        if (arrayLengthWidth != 0) {
            pushPosition();
            // Length field placeholder
            _writeValue(0, arrayLengthWidth);
            pushPosition(); // Start of vector data

            if (arrayMinLength != 0 && _value.size() < arrayMinLength) {
                errorOccurred_ = true;
            }
            if (arrayMaxLength != 0 && _value.size() > arrayMaxLength) {
                errorOccurred_ = true;
            }
        } else {
            if (arrayMaxLength != _value.size()) {
                errorOccurred_ = true;
            }
        }

        if (!hasError()) {
            // Write array/vector content
            for (auto i : _value) {
                writeValue(i, (_depl ? _depl->elementDepl_ : nullptr));
                if (hasError()) {
                    break;
                }
            }
        }

        // Write actual value of length field
        if (arrayLengthWidth != 0) {
            size_t length = getPosition() - popPosition();
            size_t position2Write = popPosition();
            _writeValueAt(uint32_t(length), arrayLengthWidth, uint32_t(position2Write));
        }

        return (*this);
    }

    template<typename KeyType_, typename ValueType_, typename HasherType_>
    OutputStream &writeValue(const std::unordered_map<KeyType_, ValueType_, HasherType_> &_value,
                             const EmptyDeployment *_depl) {
        pushPosition();
        _writeValue(static_cast<uint32_t>(0)); // Placeholder
        pushPosition(); // Start of map data

        for (auto v : _value) {
            writeValue(v.first, static_cast<EmptyDeployment *>(nullptr));
            if (hasError()) {
                return (*this);
            }

            writeValue(v.second, static_cast<EmptyDeployment *>(nullptr));
            if (hasError()) {
                return (*this);
            }
        }

        // Write number of written bytes to placeholder position
        uint32_t length = getPosition() - popPosition();
        _writeValueAt(length, popPosition());

        return (*this);
    }

    template<typename Deployment_, typename KeyType_, typename ValueType_, typename HasherType_>
    OutputStream &writeValue(const std::unordered_map<KeyType_, ValueType_, HasherType_> &_value,
                             const Deployment_ *_depl) {
        pushPosition();
        _writeValue(static_cast<uint32_t>(0)); // Placeholder
        pushPosition(); // Start of map data

        for (auto v : _value) {
            writeValue(v.first, (_depl ? _depl->key_ : nullptr));
            if (hasError()) {
                return (*this);
            }

            writeValue(v.second, (_depl ? _depl->value_ : nullptr));
            if (hasError()) {
                return (*this);
            }
        }

        // Write number of written bytes to placeholder position
        uint32_t length = uint32_t(getPosition() - popPosition());
        _writeValueAt(length, popPosition());

        return (*this);
    }

    bool hasError() const {
        return errorOccurred_;
    }

    /**
     * Reserves the given number of bytes for writing, thereby negating the need to dynamically allocate memory while writing.
     * Use this method for optimization: If possible, reserve as many bytes as you need for your data before doing any writing.
     *
     * @param numOfBytes The number of bytes that should be reserved for writing.
     */
    void reserveMemory(size_t numOfBytes) {
    }

    template<typename Type_>
    OutputStream &_writeValue(const Type_ &_value) {
        union {
            Type_ typed;
            byte_t raw[sizeof(Type_)];
        } value;
        value.typed = _value;
    #if __BYTE_ORDER == __LITTLE_ENDIAN
        byte_t *source = &value.raw[sizeof(Type_)-1];
        for (size_t i = 0; i < sizeof(Type_); ++i) {
            _writeRaw(*source--);
        }
    #else
        for (size_t i = 0; i < sizeof(Type_); ++i) {
            _writeRaw(value.raw[i]);
        }
    #endif
        return (*this);
    }

    template<typename Type_>
    void _writeValueAt(const Type_ &_value, size_t _position) {
        assert(_position + sizeof(Type_) <= payload_.size());
        union {
            Type_ typed;
            byte_t raw[sizeof(Type_)];
        } value;
        value.typed = _value;
    #if __BYTE_ORDER == __LITTLE_ENDIAN
        byte_t reordered[sizeof(Type_)];
        byte_t *source = &value.raw[sizeof(Type_)-1];
        byte_t *target = reordered;
        for (size_t i = 0; i < sizeof(Type_); ++i) {
            *target++ = *source--;
        }
        _writeRawAt(reordered, sizeof(Type_), _position);
    #else
        _writeRawAt(value.raw, sizeof(Type_), _position);
    #endif
    }

    /**
     * Fills the stream with 0-bytes to make the next value be aligned to the boundary given.
     * This means that as many 0-bytes are written to the buffer as are necessary
     * to make the next value start with the given alignment.
     *
     * @param _boundary The byte-boundary to which the next value should be aligned.
     */
    void align(const size_t _boundary) {
        assert(_boundary > 0 && _boundary <= 8 &&
            (_boundary % 2 == 0 || _boundary == 1));

        size_t mask = _boundary - 1;
        size_t necessary = ((mask - (payload_.size() & mask)) + 1) & mask;
        //Additional 0-termination, so this is 8 byte of \0
        static const byte_t eightByteZeroString[] = { 0 };
        _writeRaw(eightByteZeroString, necessary);
    }

    /**
     * Takes sizeInByte characters, starting from the character which val points to, and stores them for later writing.
     * When calling flush(), all values that were written to this stream are copied into the payload of the #Message.
     *
     * The array of characters might be created from a pointer to a given value by using a reinterpret_cast. Example:
     * @code
     * ...
     * int32_t val = 15;
     * outputMessageStream.alignForBasicType(sizeof(int32_t));
     * const char* const reinterpreted = reinterpret_cast<const char*>(&val);
     * outputMessageStream.writeValue(reinterpreted, sizeof(int32_t));
     * ...
     * @endcode
     *
     * @param _data The array of chars that should serve as input
     * @param _size The number of bytes that should be written
     * @return true if writing was successful, false otherwise.
     *
     * @see OutputMessageStream()
     * @see flush()
     */
    void _writeRaw(const byte_t &_data) {
        payload_.push_back(_data);
    }
    void _writeRaw(const byte_t *_data, const size_t _size) {
        payload_.insert(payload_.end(), _data, _data + _size);
    }
    void _writeRawAt(const byte_t *_data, const size_t _size, const size_t _position) {
        std::memcpy(&payload_[_position], _data, _size);
    }

    void _writeBom(const StringDeployment *_depl) {
        const byte_t utf8Bom[] = { 0xEF, 0xBB, 0xBF };
        const byte_t utf16LeBom[] = { 0xFF, 0xFE };
        const byte_t utf16BeBom[] = { 0xFE, 0xFF };

        if (_depl == NULL ||
            (_depl != NULL && _depl->stringEncoding_ == StringEncoding::UTF8)) {
            _writeRaw(utf8Bom, sizeof(utf8Bom));
        }
        else if (_depl->stringEncoding_ == StringEncoding::UTF16LE) {
            _writeRaw(utf16LeBom, sizeof(utf16LeBom));
        }
        else if (_depl->stringEncoding_ == StringEncoding::UTF16BE) {
            _writeRaw(utf16BeBom, sizeof(utf16BeBom));
        }
        else {
            errorOccurred_ = true;
        }
    }


    std::vector<byte_t>& get_payload(){
        return payload_;
    }

protected:
    std::vector<byte_t> payload_;

private:
    size_t getPosition() {
        return payload_.size();
    }
    void pushPosition() {
        positions_.push(payload_.size());
    }
    size_t popPosition() {
        size_t itsPosition = positions_.top();
        positions_.pop();
        return itsPosition;
    }

    bool errorOccurred_;

    std::stack<size_t> positions_;
};

}
}

#endif
