
/*
 *
 * \brief Encapsulation of an error domain.
 * 
 * An error domain is the controlling entity for ErrorCode’s error code values, and defines the
 * mapping of such error code values to textual representations.
 * This class is a literal type, and subclasses are strongly advised to be literal types as well
 * 
 * AutoSAR Adaptive Platform Release 18-10
 * Author : Even Lee
 * 
 * 2019-3-30
 * 
 */

#ifndef _ERROR_DOMAIN_H_
#define _ERROR_DOMAIN_H_

#include <cstdint>

//#include "error_code.h"


namespace ara {
namespace core {

class ErrorCode;

class ErrorDomain {

public:

    // Alias type for a unique ErrorDomain [SWS_CORE_00121]
    // TODO @liywien : is ara::core::ErrorDomain necessary in the following statement.
    using IdType = std::uint64_t; 
    // Alias type for domain error code value [SWS_CORE_00122]
    using CodeType = std::int32_t;
    // Alias type for vendor-specific supplementary data . [SWS_CORE_00123]
    using SupportDataType = std::int32_t;

public:
    ErrorDomain (ErrorDomain const &) = delete; // [SWS_CORE_00131]
    ErrorDomain (ErrorDomain &&) = delete;      // [SWS_CORE_00132]

protected:
    explicit constexpr ErrorDomain(IdType id) : m_id_type(id) { // [SWS_CORE_00133]
        // m_id_type = id;
    }
    // TODO @liyiwen : need to be virtual?
    ~ErrorDomain() = default; // [SWS_CORE_00134]
    
public:
    ErrorDomain& operator = (ErrorDomain const &) = delete; // [SWS_CORE_00135]
    ErrorDomain& operator = (ErrorDomain &&) = delete; // [SWS_CORE_00136]

    constexpr bool operator == (ErrorDomain const & other) const noexcept; // [SWS_CORE_00137]
    constexpr bool operator != (ErrorDomain const & other) const noexcept; // [SWS_CORE_00138]

public:
    // \brief Return the unique domain identifier
    // [SWS_CORE_00151]
    constexpr IdType Id() const noexcept {
        return m_id_type;
    }

    // \brief Return the name of this error domain.
    // 
    // The returned pointer remains owned by class ErrorDomain and shall not be freed by clients.
    // the name as a null-terminated string, never nullptr
    //
    // \note : TODO: -suggestion : return type could be StringView
    virtual char const * Name() const noexcept = 0; // [SWS_CORE_00152]

    // \brief Return a textual representation of the given error code.
    // 
    // The return value is undefined if the errorCode did not originate from this error domain.
    // The returned pointer remains owned by class ErrorDomain and shall not be freed by clients.
    // 
    // \note : TODO: -suggestion : return type could be StringView. Ref to [SWS_CORE_00517]
    virtual char const * Message(CodeType errorCode) const noexcept = 0; // [SWS_CORE_00153]

    // \brief Throw the given error as exception.
    // 
    // This function will determine the appropriate exception type for the given ErrorCode and throw it.
    // The thrown exception will contain the given ErrorCode.
    virtual void ThrowAsException(ara::core::ErrorCode const & errorCode) const noexcept(false) = 0; // [SWS_CORE_00154]

private:
    
    IdType m_id_type;

};

    
} // core    
} // ara

#endif // _ERROR_DOMAIN_H_

