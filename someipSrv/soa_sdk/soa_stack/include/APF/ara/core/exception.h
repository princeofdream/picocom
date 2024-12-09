
/*
 *
 * \brief Encapsulation of an error code
 * 
 * An ErrorCode contains a raw error code value and an error domain. The raw error code value is
 * specific to this error domain
 * 
 * AutoSAR Adaptive Platform Release 18-10
 * Author : Even Lee
 * 
 * 2019-3-30
 * 
 */
#ifndef _ARA_CORE_EXCEPTION_H
#define _ARA_CORE_EXCEPTION_H
#include <cstdint>
#include <exception>

#include "error_code.h"


namespace ara {
namespace core {

// \brief Base type for all AUTOSAR exception types
// \trace_id [SWS_CORE_00601]
class Exception : public std::exception {

public:

    // \brief Construct a new Exception object with a specific ErrorCode.
    //
    // \param err    : the ErrorCode
    // 
    // \trace_code [SWS_CORE_00611]
    explicit Exception(ErrorCode err) noexcept :m_error_code(err) {};

    // \brief Return the explanatory string.
    //
    // This function overrides the virtual function std::exception::what. All guarantees about the
    // lifetime of the returned pointer that are given for std::exception::what are preserved.
    // a null-terminated string
    // 
    // \trace_code [SWS_CORE_00612]
    char const* what() const noexcept override {
        return "";
    }

    // \brief  Return the embedded ErrorCode that was given to the constructor
    // 
    // \return reference to the embedded ErrorCode
    // \trace_code [SWS_CORE_00613]
    ErrorCode const & Error() const noexcept;

    ErrorCode m_error_code;
};

    
} // core    
} // ara

#endif

