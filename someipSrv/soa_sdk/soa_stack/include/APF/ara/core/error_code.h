
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

#ifndef _ERROR_CODE_H_
#define _ERROR_CODE_H_

#include <cstdint>
#include <type_traits>

#include "error_domain.h"

namespace ara {
namespace core {

template<class _Enum>
struct is_error_code_enum : std::false_type
{	// tests for error_code enumeration
};


// [SWS_CORE_00501]
class ErrorCode {

public:

    // \brief Construct a new ErrorCode instance with parameters.
    //
    // This constructor does not participate in overload resolution unless EnumT is an enum type.
    // The lifetime of the text pointed to by userMessage must exceed that of all uses of Error
    // Code::UserMessage() on this instance 
    // 
    // \temp_param EnumT : an enum type that contains error code values.
    // 
    // \param e    : a domain-specific error code value
    // \param data : optional vendor-specific supplementary error context data
    // \param userMessage : an optional user-defined custom static message text (null-terminated)
    // 
    // \trace_id [SWS_CORE_00511]
    // \note : TODO: -issue  [SWS_CORE_00512] is almost the same except for parameter order. It might be a mistake.
	//
	// \ref to  system_error of C++ lib
	// 
	// template<typename _ErrorCodeEnum, typename = typename
	//     enable_if<is_error_code_enum<_ErrorCodeEnum>::value>::type>
    //  error_code(_ErrorCodeEnum __e) noexcept
    //  { *this = make_error_code(__e); }
	//
    template <typename EnumT, typename = std::enable_if_t<is_error_code_enum<EnumT>::value>>
    constexpr ErrorCode(EnumT e , ErrorDomain::SupportDataType data = 0, char const * userMessage = nullptr) noexcept
		: ErrorCode(MakeErrorCode(e, data, userMessage))  // find MakeErrorCode by ADL
	{
        ;
    }
	
	// Update 
	template <typename EnumT, typename = std::enable_if_t<is_error_code_enum<EnumT>::value>>
    constexpr ErrorCode(EnumT e, char const * userMessage, ErrorDomain::SupportDataType data = 0) noexcept
		: ErrorCode(MakeErrorCode(e, data, userMessage))  // find MakeErrorCode by ADL
	{
        ;
    }

    // \brief Construct a new ErrorCode instance with parameters.
    //
    // The lifetime of the text pointed to by userMessage must exceed that of all uses of Error
    // Code::UserMessage() on this instance.
    //  
    // \param value : a domain-specific error code value
    // \param domain: the ErrorDomain associated with value
    // \param data  : optional vendor-specific supplementary error context data
    // \param userMessage : an optional user-defined custom static message text (null-terminated)
    // 
    // \trace_id [SWS_CORE_00513]
    constexpr ErrorCode(
		ErrorDomain::CodeType value, 
		ErrorDomain const & domain, 
		ErrorDomain::SupportDataType data = 0, 
		char const * userMessage = nullptr) noexcept :
        m_code_type(value), 
        m_error_domain(&domain),
        m_support_data_type(data),
        m_user_message(userMessage) 
	{
    }

    // \brief  Return the raw error code value.
    // the raw error code value
    // \trace_id [SWS_CORE_00514]
    constexpr ErrorDomain::CodeType Value() const noexcept {
        return m_code_type;
    }

    // \brief  Return the domain with which this ErrorCode is associated
    // the ErrorDomain
    // \trace_id [SWS_CORE_00515]
    constexpr ErrorDomain const & Domain() const noexcept {
        return *m_error_domain;
    }

    // \brief  Return the supplementary error context data
    // The meaning of the returned value is implementation-defined.
    // the supplementary error context data
    // \trace_id [SWS_CORE_00516]
    constexpr ErrorDomain::SupportDataType SupportData() const noexcept {
        return m_support_data_type;
    }

    // \brief  Return the user-specified message text.
    // the user-specified message text, or an empty string if none was given
    //
	// TODO : @liyiwen -> Change to StringView
	// \trace_id [SWS_CORE_00517]
	constexpr char const * UserMessage() const noexcept {
		return m_user_message;
	}

    // \brief  Return a textual representation of this ErrorCode.
    // the error message text
    //
    // \trace_id [SWS_CORE_00518]
	// TODO : @liyiwen -> Change to StringView
	char const * Message() const noexcept {
		return m_error_domain->Message(m_code_type);
	}

private:
    ErrorDomain::CodeType         m_code_type;              // type-erased error code;
    ErrorDomain const *           m_error_domain; 
	ErrorDomain::SupportDataType  m_support_data_type;
	char const *                  m_user_message = nullptr; 
};

    
} // namespace core    
} // namespace ara


#endif // _ERROR_CODE_H_