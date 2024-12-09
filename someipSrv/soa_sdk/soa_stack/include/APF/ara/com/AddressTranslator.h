#ifndef _ARA_COM_ADDRESS_TRANSLATOR_H
#define _ARA_COM_ADDRESS_TRANSLATOR_H
#include <map>
#include <iostream>
#include <vector>
#include <map>
#include <mutex>
#include <memory>

namespace ara {
namespace com {
    typedef uint16_t service_t;
    typedef uint16_t method_t;
    typedef uint16_t event_t;

    typedef uint16_t instance_t;
    typedef uint16_t eventgroup_t;

    typedef uint8_t major_version_t;
    typedef uint32_t minor_version_t;
    class Someip_Address {
    public:
        Someip_Address();
        Someip_Address(const service_t _service, const instance_t _instance,
            major_version_t _major_version = 0, minor_version_t _minor_version = 0);

        bool operator==(const Someip_Address& _other) const;
        bool operator!=(const Someip_Address& _other) const;
        bool operator<(const Someip_Address& _other) const;

        const service_t& getService() const;

        const instance_t& getInstance() const;

        const major_version_t& getMajorVersion() const;

        const minor_version_t& getMinorVersion() const;

    private:
        service_t service_;
        instance_t instance_;
        major_version_t major_version_;
        minor_version_t minor_version_;

        friend std::ostream& operator<<(std::ostream& _out, const Someip_Address& _address);
    };

    class ARA_Address {
    public:
        ARA_Address() = default;
        ARA_Address(const std::string& _address);
        ARA_Address(const std::string& _domain,
            const std::string& _interface,
            const std::string& _instance);
        ARA_Address(const ARA_Address& _source);
        virtual ~ARA_Address();

        bool operator==(const ARA_Address& _other) const;
        bool operator!=(const ARA_Address& _other) const;
        bool operator<(const ARA_Address& _other) const;

        std::string getAddress() const;
        void setAddress(const std::string& _address);

        const std::string& getDomain() const;
        void setDomain(const std::string& _domain);

        const std::string& getInterface() const;
        void setInterface(const std::string& _interface);

        const std::string& getInstance() const;
        void setInstance(const std::string& _instance);

    private:
        std::string domain_;
        std::string interface_;
        std::string instance_;

        friend std::ostream& operator<<(std::ostream& _out, const ARA_Address& _address);
    };

    class AddressTranslator {
    public:
        static std::shared_ptr<AddressTranslator> get();
        AddressTranslator();
        bool translate(const std::string& _key, Someip_Address& _value);
        bool translate(const Someip_Address& _key, std::string& _value);
        void insert(const std::string& _address,
            service_t _service, instance_t _instance,
            major_version_t _major, minor_version_t _minor);
        std::map<std::string,Someip_Address> GetAllAddressByInterface(const std::string& _interface);
        std::vector<std::string> GetAllInstanceAddressByInterface(const std::string& _interface);
    private:
        std::map<std::string, Someip_Address> forwards_;
        std::map<Someip_Address, std::string> backwards_;
        std::mutex mutex_;
    };
}
}
#endif