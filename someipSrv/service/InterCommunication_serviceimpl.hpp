#ifndef _INTER_COMMUNICATION_IMPL_H_
#define _INTER_COMMUNICATION_IMPL_H_

#include <InterCommunication/InterCommunication_skeleton.hpp>

void dump_hex_strip(const uint8_t* data, size_t size, size_t display,const char *split);
void dump_hex(const uint8_t* data, size_t size, const char *split);
void dump_hex_vec_strip(std::vector<uint8_t> vec_data, size_t display, const char *split);
void dump_hex_vec(std::vector<uint8_t> vec_data, const char *split);
extern int fdRead;

class InterCommunication_serviceimpl :public v1::ara_api::InterCommunication::skeleton::InterCommunicationSkeleton {
public:
    InterCommunication_serviceimpl(ara::com::InstanceIdentifier instanceID, ara::com::MethodCallProcessingMode mode = ara::com::MethodCallProcessingMode::kEvent) :v1::ara_api::InterCommunication::skeleton::InterCommunicationSkeleton(instanceID, mode)
    {
    }

    InterCommunication_serviceimpl(ara::core::InstanceSpecifier instanceSpec, ara::com::MethodCallProcessingMode mode = ara::com::MethodCallProcessingMode::kEvent) :v1::ara_api::InterCommunication::skeleton::InterCommunicationSkeleton(instanceSpec, mode)
    {
    }

    InterCommunication_serviceimpl(ara::com::InstanceIdentifierContainer instanceIDs, ara::com::MethodCallProcessingMode mode) :InterCommunicationSkeleton(instanceIDs, mode)
    {
    }

    void InterRequest(v1::ara_api::InterCommunication::Array_InterComm& Array_InterComm_name);
    ara::core::Future<v1::ara_api::InterCommunication::Array_InterComm> InterRequestRR(v1::ara_api::InterCommunication::Array_InterComm& RequestRR);
};
#endif //_INTER_COMMUNICATION_IMPL_H_
