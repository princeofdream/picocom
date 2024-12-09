#ifndef ARA_MESSAGE_HPP
#define ARA_MESSAGE_HPP
#if _WIN32
#define KTSOMEIP_EXPORT __declspec(dllexport)
#else
#define KTSOMEIP_EXPORT
#endif
#include <cstdint>
#include <memory>
#include "AppSpecialMessage.hpp"

#include "Handler.hpp"

namespace ara {
    typedef uint32_t length_t;
    typedef uint16_t service_t;
    typedef uint16_t method_t;
    typedef uint16_t client_id_t;
    typedef uint16_t instance_id_t;
    typedef uint16_t session_id_t;
    typedef uint32_t message_length_t;
    typedef uint16_t event_t;
    typedef uint16_t check_result_t;
    typedef uint16_t call_back_id;
    typedef uint8_t byte_t;
#ifdef _WIN32
    typedef std::uint32_t uid_t;
    typedef std::uint32_t gid_t;
#else
    typedef ::uid_t uid_t;
    typedef ::uid_t gid_t;
#endif
    class SomeIPMessageIns;
    class  Message {
    public:
        KTSOMEIP_EXPORT Message();

        KTSOMEIP_EXPORT ~Message();

        KTSOMEIP_EXPORT Message(const std::shared_ptr<kt_someip::AppMsgSomeipSendCmd> &_source);

        KTSOMEIP_EXPORT Message(const std::shared_ptr<SomeIPMessageIns> &_source);

        KTSOMEIP_EXPORT Message(const Message& _source);

        KTSOMEIP_EXPORT Message(Message&& _source);

        KTSOMEIP_EXPORT Message& operator=(const Message& _source);
        
        KTSOMEIP_EXPORT bool operator < (const Message& _source) const;

        KTSOMEIP_EXPORT bool operator== (const Message& _source) const;

        KTSOMEIP_EXPORT Message& operator=(Message&& _source);

        KTSOMEIP_EXPORT Message& operator=(const std::shared_ptr<SomeIPMessageIns> &_source);

        KTSOMEIP_EXPORT void setPayloadData(const byte_t* data, message_length_t length);

        KTSOMEIP_EXPORT void setPayloadData(const std::vector<byte_t>& _data);

        /*
        \brief :transport as tcp if reliable is true 
        */
        KTSOMEIP_EXPORT static Message createRequestMessage(bool reliable = false);

        KTSOMEIP_EXPORT Message createResponseMessage() const;

        KTSOMEIP_EXPORT Message createErrorResponseMessage(apf::someip::MessageStandardHeader::ReturnCode _return_code) const;

        KTSOMEIP_EXPORT void setService(service_t _service);

        KTSOMEIP_EXPORT void setInstance(instance_id_t _instance);

        KTSOMEIP_EXPORT void setMethod(method_t _method);

        KTSOMEIP_EXPORT void setSession(session_id_t _session);

        KTSOMEIP_EXPORT service_t getService();

        KTSOMEIP_EXPORT instance_id_t  getInstance();

        KTSOMEIP_EXPORT method_t getMethod();

        KTSOMEIP_EXPORT session_id_t getSession();

        KTSOMEIP_EXPORT client_id_t getClientId();

        KTSOMEIP_EXPORT void setClientId(client_id_t _client_id);

        KTSOMEIP_EXPORT static Message createNotificationMessage(const service_t service_ ,const instance_id_t instance_,
            const event_t _event, bool _reliable = false);

        KTSOMEIP_EXPORT std::shared_ptr<SomeIPMessageIns>& getMessage();

        KTSOMEIP_EXPORT byte_t* getBodyData() const;

        KTSOMEIP_EXPORT message_length_t getBodyLength() const;
        /*
        0: E2E_OK
        1: E2E_WRONG_CRC
        2: E2E_ERROR
        */
        KTSOMEIP_EXPORT check_result_t getCheckResult()const;

        KTSOMEIP_EXPORT void setInterfaceVersion(major_t major);

        KTSOMEIP_EXPORT major_t getInterfaceVersion();
    public:
        const message_length_t E2E_OK = 0;
        const message_length_t E2E_WRONG_CRC = 1;
        const message_length_t E2E_ERROR = 2;
        const message_length_t E2E_ERROR_METHOD = 3;

    private:
        std::shared_ptr<SomeIPMessageIns> message_;
    };

    class SomeIPMessageIns {
    public:
        SomeIPMessageIns();
        ~SomeIPMessageIns();

        uint16_t getInstance();
        void setInstance(uint16_t _instance);
        std::shared_ptr<apf::someip::SomeIpMessage<>> &getMessage();
        void setMessage(std::shared_ptr<apf::someip::SomeIpMessage<> > _message);
    private:
        uint16_t m_instance;
        std::shared_ptr<apf::someip::SomeIpMessage<>> m_message;
    };
};

#endif
