#ifndef _ARA_COM_INSTANCE_IDENTIFYER_H_
#define _ARA_COM_INSTANCE_IDENTIFYER_H_
#include "ara/core/string_view.h"
#include "ara/core/vector.h"

namespace ara {
    namespace com {
        class InstanceIdentifier {
        public:
            static const InstanceIdentifier Any;

            explicit InstanceIdentifier(ara::core::StringView value) {
#if nssv_CPP17_OR_GREATER
                m_identify = value;
#else
                m_identify = value.to_string();
#endif
            }
            ara::core::StringView toString() const {
                return m_identify;
            }
            bool operator==(const InstanceIdentifier& other) const {
                if (m_identify == other.m_identify)
                    return true;
                return false;
            }
            bool operator<(const InstanceIdentifier& other) const {
                if (m_identify < other.m_identify)
                    return true;
                return false;
            }
            InstanceIdentifier& operator=(const InstanceIdentifier& other) {
                m_identify = other.m_identify;
                return *this;
            }
        private:
            std::string m_identify;
        };

        using InstanceIdentifierContainer = ara::core::Vector<InstanceIdentifier>;
    }
}
#endif