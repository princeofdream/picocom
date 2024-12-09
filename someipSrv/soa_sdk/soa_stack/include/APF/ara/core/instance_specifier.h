#ifndef _ARA_CORE_INSTANCE_SPECIFIER_H
#define _ARA_CORE_INSTANCE_SPECIFIER_H
#include "string_view.h"
#include "string.h"
namespace ara {
namespace core {
    class InstanceSpecifier {
	public:
        explicit InstanceSpecifier(ara::core::StringView value) {
            //std::string s = value.to_string();
#if nssv_CPP17_OR_GREATER
            m_instance_specifier = value;
#else
            m_instance_specifier = value.to_string();
#endif
        }
        ara::core::StringView toString() const {
            return m_instance_specifier;
        }
        bool operator==(const InstanceSpecifier& other) const {
            if (m_instance_specifier == other.m_instance_specifier)
                return true;
            return false;
        }
        bool operator<(const InstanceSpecifier& other) const {
            if (m_instance_specifier < other.m_instance_specifier)
                return true;
            return false;
        }
        InstanceSpecifier& operator=(const InstanceSpecifier& other) {
            m_instance_specifier = other.m_instance_specifier;
            return *this;
        }
    private:
        ara::core::String m_instance_specifier;
	};
}
}
#endif