#include <iostream>
#include <random>
#include <unistd.h>
#include <fcntl.h>

#include "InterCommunication_serviceimpl.hpp"
#include "Intercomm.h"

#define ENABLE_EVENT    1		// fields::InterEvent
#define ENABLE_RRMETHOD 1		// InterRequest


using namespace v1::ara_api::InterCommunication;
std::unique_ptr<skeleton::InterCommunicationSkeleton> myInterCommunicationSkeleton;

IntercommSrv::IntercommSrv()
{
    srvReady = 0;
}

IntercommSrv::~IntercommSrv()
{
    soaSlogI("%s exit", __func__);
}

int IntercommSrv::Init()
{
	IntercommSkeleton->Init();
    if(!ara::Runtime::getInstance()->init()) {
        soaSlogW("ara::Runtime::getInstance()->init() failed!!!");
        return 0;
    }
    startSrvThread();

    return 0;
}

int IntercommSrv::MainThread(int val) {
    std::string specifier(InterCommunicationIdentifier::ServiceIdentifier.toString());
    IntercommSkeleton = std::make_unique<InterCommunication_serviceimpl>(ara::core::InstanceSpecifier(specifier), ara::com::MethodCallProcessingMode::kEvent);

    ara::core::Result<void> offer_result = IntercommSkeleton->OfferService();
    if (!offer_result.HasValue())
    {
        soaSlogW("Offer service error: %s",offer_result.Error().Message());
        return -EINVAL;
    }

    srvReady = 1;
    while (1)
    {
#if ENABLE_EVENT
            {
                skeleton::events::InterEvent::SampleType eventsVal;
                ara::core::String eventsStrVal = " field Event id " + std::to_string(20);
                eventsVal.setStringValue(eventsStrVal);
                soaSlogI("[ srv ][ Event    ] Sending event [ %s ].", eventsVal.getStringValue().data());
                myInterCommunicationSkeleton->InterEvent.Send(eventsVal);
            }
#if 0
            {
                skeleton::events::InterEventArr::SampleType eventsVal;
                std::vector<Uint8_Rawdata> vec;

                if (fdRead > 0) {
                    int rlen;
                    uint8_t buf[2];
                    lseek(fdRead, 0, SEEK_SET);
                    while (1) {
                        memset(buf, 0x0, sizeof(buf));
                        rlen = read(fdRead, &buf, sizeof(buf[0]));
                        if (rlen <= 0) {
                            break;
                        }
                        vec.push_back(buf[0]);
                    }
                }
                dump_hex_vec_strip(vec, 128, "Event Send");
                eventsVal.setValue(vec);
                myInterCommunicationSkeleton->InterEventArr.Send(eventsVal);
            }
#endif
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
    return 0;
}

int IntercommSrv::startSrvThread() {
    srvThread = std::thread(std::bind(&IntercommSrv::MainThread, this, std::placeholders::_1), 0);
    srvThread.detach();
    soaSlogI("IntercommSkeleton init done!");
    return 0;
}

