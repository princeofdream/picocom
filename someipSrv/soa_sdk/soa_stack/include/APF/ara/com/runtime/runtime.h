#ifndef _ARA_COM_RUNTIME_RUNTIME_H_
#define _ARA_COM_RUNTIME_RUNTIME_H_
#include "ara/core/instance_specifier.h"
#include "ara/com/instance_identifyer.h"
#include "ara/core/result.h"
#include "ara/com/AddressTranslator.h"

namespace ara {
    namespace com {
        namespace runtime {
            /*
            * [SWS_CM_00118] in 20.11
            * In case ResolveInstanceIDs() fails the error code ComErrc::kInstanceIDCouldNotBeResolved shall be returned
            */
            ara::core::Result<ara::com::InstanceIdentifierContainer> ResolveInstanceIDs(ara::core::InstanceSpecifier modelName);
            class UtilityTempClass {
                friend ara::core::Result<ara::com::InstanceIdentifierContainer> ResolveInstanceIDs(ara::core::InstanceSpecifier modelName) {
                    std::vector<std::string> instances = ara::com::AddressTranslator::get()->GetAllInstanceAddressByInterface(std::string(modelName.toString()));
                    if (instances.size() > 0)
                    {
                        InstanceIdentifierContainer identifier_container;
                        for (auto& instance : instances)
                        {
                            identifier_container.push_back(InstanceIdentifier(instance));
                        }
                        return identifier_container;
                    }
                    else
                    {
                        return ara::core::Result<ara::com::InstanceIdentifierContainer>::FromError(ara::com::ComErrc::kInstanceIDCouldNotBeResolved);
                    }
                }
            };
        }
    }
}
#endif