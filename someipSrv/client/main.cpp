#include <InterCommunication/InterCommunication_proxy.hpp>
#include <iostream>
#include <random>

#define ENABLE_EVENT    1		// fields::InterEvent
#define ENABLE_GETTER   0		// fields::InterField
#define ENABLE_SETTER   0		// fields::InterField
#define ENABLE_NOTIFY   1		// fields::InterField
#define ENABLE_RRMETHOD 0		// InterRequestRR

std::string str_context;

using namespace v1::ara_api::InterCommunication;

std::unique_ptr<proxy::InterCommunicationProxy> myInterCommunicationProxy;
#if ENABLE_NOTIFY
std::deque<ara::com::SamplePtr<proxy::fields::InterField::FieldType>> lastNActiveSamples_InterField;
std::deque<ara::com::SamplePtr<proxy::fields::InterFieldArr::FieldType>> lastNActiveSamples_InterFieldArr;
#endif
#if ENABLE_EVENT
std::deque<ara::com::SamplePtr<proxy::events::InterEvent::SampleType>> lastNActiveSamples_InterEvent;
std::deque<ara::com::SamplePtr<proxy::events::InterEventArr::SampleType>> lastNActiveSamples_InterEventArr;
#endif

void dump_hex_strip(const uint8_t* data, size_t size, size_t display,const char *split)
{
    size_t loop;
    size_t display_sz;
    char value[1024];

    display_sz = display;
    if (display == 0 || display >= size) {
        display_sz = size;
    }

    loop = 0;
    memset(value, 0x0, sizeof(value));
    while (loop < size) {
        if (loop == 0) {
            sprintf(value,"%s","\n");
            sprintf(value, "%s%s",value, "----------------------------------------------");
            // sprintf(value, "%s%s", value, "\n");
            printf("%s\n", value);
            memset(value, 0x0, sizeof(value));
            sprintf(value, "%s %02x0 | ", split, loop / 16);
        }

        sprintf(value, "%s %02x", value, data[loop]);

        if (loop % 16 == 15) {
            // sprintf(value, "%s%s", value, "\n");
            if (loop/16 <= display_sz/16 || loop/16+2 >= size/16) {
                printf("%s\n", value);
            }
            if (loop/16-1 == display_sz/16 && loop/16+2 != size/16) {
                printf("...\n");
            }
            memset(value, 0x0, sizeof(value));
            if (value[0] == 0x0) {
                sprintf(value, "%s %02x0 | ", split, loop / 16 + 1);
            } else {
                sprintf(value, "%s%s %02x0 | ", value, split, loop / 16 + 1);
            }
        } else if (loop % 8 == 7) {
            sprintf(value, "%s%s", value, "  ");
        }
        loop++;
    }
    // sprintf(value, "%s%s", value, "\n");
    printf("%s\n", value);
}

void dump_hex(const uint8_t* data, size_t size, const char *split)
{
    dump_hex_strip(data, size, size, split);
}

void dump_hex_vec_strip(std::vector<uint8_t> vec_data, size_t display, const char *split)
{
    uint8_t* data = vec_data.data();
    size_t len = vec_data.size();

    dump_hex_strip(data, len, display, split);
}

void dump_hex_vec(std::vector<uint8_t> vec_data, const char *split)
{
    uint8_t* data = vec_data.data();
    size_t len = vec_data.size();

    dump_hex_strip(data, len, len, split);
}

#if ENABLE_NOTIFY
void InterFieldNotifierHandler() {
    ara::core::Result<size_t> result = myInterCommunicationProxy->InterField.GetNewSamples(
        [](ara::com::SamplePtr<proxy::fields::InterField::FieldType> samplePtr) {
            if (samplePtr) {
                lastNActiveSamples_InterField.push_back(std::move(samplePtr));
                if (lastNActiveSamples_InterField.size() > 10)
                    lastNActiveSamples_InterField.pop_front();
            }
        }, 10);
    if (result.HasValue()) {
        ara::com::SamplePtr<proxy::fields::InterField::FieldType> sample = std::move(lastNActiveSamples_InterField.front());
        proxy::fields::InterField::FieldType recvNotify = *sample;
        soaSlogI("[ cli ][ Notifier ] Recv notifier: [ ID: %d, data ID: %d, Data: %s ].",
            (uint8_t)recvNotify.getUint8_InterFieldID(),
            (uint8_t)recvNotify.getStruct_InterFieldData().getUint8_DataID(),
            recvNotify.getStruct_InterFieldData().getString_DataMsg().c_str());
        while (lastNActiveSamples_InterField.size() > 0)
        {
            lastNActiveSamples_InterField.pop_front();
        }
    }
    else {
        ara::core::ErrorCode const& ec = result.Error();
        std::cerr << "[ cli ][ Notifier ] InterField.GetNewSamples failed," << ec.Message() << std::endl;
    }
}
#endif

#if ENABLE_EVENT
void InterEventHandler()
{
    ara::core::Result<size_t> num = myInterCommunicationProxy->InterEvent.GetNewSamples(
        [](ara::com::SamplePtr<proxy::events::InterEvent::SampleType> samplePtr) {
            if (samplePtr) {
                lastNActiveSamples_InterEvent.push_back(std::move(samplePtr));
                if (lastNActiveSamples_InterEvent.size() > 10) {
                    lastNActiveSamples_InterEvent.pop_front();
                }
            }
        }, 10);
    if (num.HasValue()) {
        ara::com::SamplePtr<proxy::events::InterEvent::SampleType> sample = std::move(lastNActiveSamples_InterEvent.front());
        soaSlogI("[ cli ][ Event    ] Recv event: [ %s ].", sample->getStringValue().data());
        while (lastNActiveSamples_InterEvent.size() > 0) {
            lastNActiveSamples_InterEvent.pop_front();
        }
    } else {
        ara::core::ErrorCode const& ec = num.Error();
        std::cerr << "[ cli ][ Event    ] InterEvent.GetNewSamples failed," << ec.Message() << std::endl;
    }
}

void InterEventArrHandler()
{
    ara::core::Result<size_t> num = myInterCommunicationProxy->InterEventArr.GetNewSamples(
        [](ara::com::SamplePtr<proxy::events::InterEventArr::SampleType> samplePtr) {
            if (samplePtr) {
                lastNActiveSamples_InterEventArr.push_back(std::move(samplePtr));
                if (lastNActiveSamples_InterEventArr.size() > 10) {
                    lastNActiveSamples_InterEventArr.pop_front();
                }
            }
        }, 10);
    if (num.HasValue()) {
        ara::com::SamplePtr<proxy::events::InterEventArr::SampleType> sample = std::move(lastNActiveSamples_InterEventArr.front());
        std::vector<Uint8_Rawdata> vec;
        vec = (*sample).getValue();
        dump_hex_vec_strip(vec, 128, "Event Recv");
    } else {
        ara::core::ErrorCode const& ec = num.Error();
        std::cerr << "[ cli ][ EventArr    ] InterEventArr.GetNewSamples failed," << ec.Message() << std::endl;
    }
}
#endif

static bool g_availablestate = false;

void StartFindService_cb(service_t _service_id, instance_t _instance_id, major_t _major_id, bool is_available)
{
    g_availablestate = is_available;
    soaSlogI("    StartFindService_cb: %d", is_available);
    if(is_available == false || is_available == true)
    {
        if(is_available){
            soaSlogW("Service [ 0x%04x ] server is available.", _service_id);
        }
        else{
            // std::cerr << _service_id << " Server is Unavailable." << std::endl;
            soaSlogW("Server is Unavailable.");
        }
    }
}

int main()
{
	myInterCommunicationProxy->Init();
    if (!ara::Runtime::getInstance()->init()) {
        soaSlogW("ara::Runtime::getInstance()->init() failed!!!");
        return 0;
    }


    ara::ServiceStatusCallback get_cb = StartFindService_cb;
    ara::core::Result<ara::com::ServiceHandleContainer<proxy::InterCommunicationProxy::HandleType>> handles = proxy::InterCommunicationProxy::StartFindService(get_cb);
    while(!g_availablestate) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // soaSlogD("[DEBUG] g_availablestate: %d", g_availablestate);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    soaSlogI("InterCommunication service has been found...");
    myInterCommunicationProxy = std::make_unique<proxy::InterCommunicationProxy>(handles.Value()[0]);

#if ENABLE_NOTIFY
    // Field Notifier Subscribe
    myInterCommunicationProxy->InterField.Subscribe(10);
    myInterCommunicationProxy->InterField.SetReceiveHandler(InterFieldNotifierHandler);
#endif

#if ENABLE_EVENT
    // event
    myInterCommunicationProxy->InterEvent.Subscribe(10);
    myInterCommunicationProxy->InterEvent.SetReceiveHandler(InterEventHandler);
    myInterCommunicationProxy->InterEventArr.Subscribe(10);
    myInterCommunicationProxy->InterEventArr.SetReceiveHandler(InterEventArrHandler);
#endif

    static std::random_device s_dev;
    static std::mt19937 s_gen(s_dev());
    std::uniform_int_distribution<int> mGenRandomVal(0, 0x00FFFFFF);
    uint32_t randomVal = mGenRandomVal(s_gen);

    std::string basicStrVal = "Syncore cli -> srv";
    while (true)
    {
#if ENABLE_RRMETHOD
        // method test
        {
#if 1
            // FF Method
            {
                Array_InterComm FFArrReq;
                std::string FFStrVal;
                std::vector<Uint8_Rawdata> FFVecVal;

                // string --> vector --> array
                FFStrVal = basicStrVal + " FF Method Request " + std::to_string(randomVal);
                FFVecVal.assign(FFStrVal.begin(), FFStrVal.end());
                FFArrReq.setValue(FFVecVal);

                soaSlogI("[ cli ][ Method   ] Sending FF Request [ %s ].", FFStrVal.c_str());
                myInterCommunicationProxy->InterRequest(FFArrReq);
            }
#endif
            // RR Method
            {
                Array_InterComm RRArrReq;
                std::vector<Uint8_Rawdata> RRVecVal;
                std::string RRStrVal;

                RRStrVal = basicStrVal + " RR Method Request " + std::to_string(randomVal);
                RRVecVal.assign(RRStrVal.begin(), RRStrVal.end());
                RRArrReq.setValue(RRVecVal);

                soaSlogI("[ cli ][ Method   ] Sending RR request [ %s ].", RRStrVal.c_str());
                ara::core::Future<Array_InterComm> result = myInterCommunicationProxy->InterRequestRR(RRArrReq);
                if (result.wait_for(std::chrono::duration<long long, std::milli>(3000)) == ara::core::future_status::ready) {
                    Array_InterComm value = result.get();
                    std::vector<Uint8_Rawdata> vec_ret = value.getValue();
#if 0
                    string str_val;
                    str_val.assign(vec_ret.begin(), vec_ret.end());
                    soaSlogI("[ cli ][ Method   ] method InterRequestRR() result-> [ %s ].", str_val.c_str());
#else
                    dump_hex_vec_strip(vec_ret, 128, "recv");
#endif
                }
                else {
                    soaSlogW("[ cli ][ Method   ] Method result timeout ...");
                }
            }
        }
#endif
#if ENABLE_GETTER
        {
            ara::core::Future<proxy::fields::InterField::FieldType> result = myInterCommunicationProxy->InterField.Get();
            if (result.wait_for(std::chrono::duration<long long, std::milli>(300)) == ara::core::future_status::ready) {
                proxy::fields::InterField::FieldType recvGetter = result.get();
                soaSlogI("[ cli ][ Getter   ] get [ ID: %d, data ID: %d, Data: %s ]",
                    (uint8_t)recvGetter.getUint8_InterFieldID(),
                    (uint8_t)recvGetter.getStruct_InterFieldData().getUint8_DataID(),
                    recvGetter.getStruct_InterFieldData().getString_DataMsg().c_str());
            } else {
                soaSlogW("[ cli ][ Getter   ] getter timeout ...");
            }
        }
#endif
#if ENABLE_SETTER
        {
            proxy::fields::InterField::FieldType sendSetter;
            Struct_InterFieldData sendSetterMsg;
            std::string sendSetterMsgStrVal = basicStrVal +
                " Setter " + std::to_string(randomVal + 40);

            sendSetter.setUint8_InterFieldID(randomVal + 30);
            sendSetterMsg.setUint8_DataID(randomVal + 40);
            sendSetterMsg.setString_DataMsg(sendSetterMsgStrVal);
            sendSetter.setStruct_InterFieldData(sendSetterMsg);

            myInterCommunicationProxy->InterField.Set(sendSetter);
            soaSlogI("[ cli ][ Setter   ] set [ ID: %d, data ID: %d, Data: %s ]",
                (uint8_t)sendSetter.getUint8_InterFieldID(),
                (uint8_t)sendSetter.getStruct_InterFieldData().getUint8_DataID(),
                sendSetter.getStruct_InterFieldData().getString_DataMsg().c_str());
        }
#endif

        randomVal++;
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    return 0;
}

