#ifndef ARA_MESSAGE_HPP
#define ARA_MESSAGE_HPP
#if _WIN32
#define VSOMEIP_EXPORT __declspec(dllexport)
#else
#define VSOMEIP_EXPORT
#endif
#include <cstdint>
#include <memory>
#include <vsomeip/vsomeip.hpp>
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
    class  Message {
    public:
        VSOMEIP_EXPORT Message();

        VSOMEIP_EXPORT ~Message();

        VSOMEIP_EXPORT Message(const std::shared_ptr<vsomeip::message> &_source);

        VSOMEIP_EXPORT Message(const Message& _source);

        VSOMEIP_EXPORT Message(Message&& _source);

        VSOMEIP_EXPORT Message& operator=(const Message& _source);
        
        VSOMEIP_EXPORT bool operator < (const Message& _source) const;

        VSOMEIP_EXPORT Message& operator=(Message&& _source);

        VSOMEIP_EXPORT void setPayloadData(const byte_t* data, message_length_t length);

        VSOMEIP_EXPORT void setPayloadData(const std::vector<byte_t>& _data);

        /*
        \brief :transport as tcp if reliable is true 
        */
        VSOMEIP_EXPORT static Message createRequestMessage(bool reliable = false);

        VSOMEIP_EXPORT Message createResponseMessage() const;

        VSOMEIP_EXPORT void setService(service_t _service);

        VSOMEIP_EXPORT void setInstance(instance_id_t _instance);

        VSOMEIP_EXPORT void setMethod(method_t _method);

        VSOMEIP_EXPORT void setSession(session_id_t _session);

        VSOMEIP_EXPORT service_t getService();

        VSOMEIP_EXPORT instance_id_t  getInstance();

        VSOMEIP_EXPORT method_t getMethod();

        VSOMEIP_EXPORT session_id_t getSession();

        VSOMEIP_EXPORT client_id_t getClientId();

        VSOMEIP_EXPORT void setClientId(client_id_t _client_id);

        VSOMEIP_EXPORT static Message createNotificationMessage(const service_t service_ ,const instance_id_t instance_,
            const event_t _event, bool _reliable = false);

        std::shared_ptr<vsomeip::message>& getMessage();

        VSOMEIP_EXPORT byte_t* getBodyData() const;

        VSOMEIP_EXPORT message_length_t getBodyLength() const;
        /*
        0: E2E_OK
        1: E2E_WRONG_CRC
        2: E2E_ERROR
        */
        VSOMEIP_EXPORT check_result_t getCheckResult()const;

    private:

        std::shared_ptr<vsomeip::message> message_;

    };

};

#endif
