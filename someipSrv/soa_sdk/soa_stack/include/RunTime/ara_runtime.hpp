#ifndef ARA_RUNTIME_HPP
#define ARA_RUNTIME_HPP
#include<memory>
#include <string.h>

#include <functional>
#include "ara_message.hpp"
#ifdef CONFIG_ENABLE_PHM
#include "phm/phm_message.h"
#endif

#if _WIN32
#define KTSOMEIP_EXPORT __declspec(dllexport)
#define KTSOMEIP_EXPORT_CLASS_EXPLICIT

#if KTSOMEIP_DLL_COMPILATION
#define KTSOMEIP_IMPORT_EXPORT __declspec(dllexport)
#else
#define KTSOMEIP_IMPORT_EXPORT __declspec(dllimport)
#endif

#if KTSOMEIP_DLL_COMPILATION_CONFIG
#define KTSOMEIP_IMPORT_EXPORT_CONFIG __declspec(dllexport)
#else
#define KTSOMEIP_IMPORT_EXPORT_CONFIG __declspec(dllimport)
#endif
#else
#define KTSOMEIP_EXPORT
#define KTSOMEIP_IMPORT_EXPORT
#define KTSOMEIP_IMPORT_EXPORT_CONFIG
#endif

namespace ara {
	template <typename... T>

	void ignore_unused(T const& ...) {}
	enum class EServiceStatus {
		OFF,
		ON
		
	};
	typedef std::function<void (Message) > call_back;
	typedef std::function<void (service_t, instance_t, major_t, bool) > ServiceStatusCallback;
	
	


	class KTSOMEIP_EXPORT Runtime {
	public:
		virtual ~Runtime() {}
		static std::shared_ptr<Runtime> getInstance();

		virtual bool init() = 0;

		virtual void offer_service(uint16_t _service_id, uint16_t _instance_id, major_t _major = kt_someip::ANY_MAJOR, minor_t _minor = kt_someip::ANY_MINOR) = 0;
		
		virtual void stop_offer_service(uint16_t _service_id, uint16_t _instance_id, major_t _major = kt_someip::ANY_MAJOR, minor_t _minor = kt_someip::ANY_MINOR) = 0;
		
		virtual bool find_service(uint16_t _service_id, uint16_t _instance_id, major_t _major_id = kt_someip::ANY_MAJOR) = 0;

		virtual void send_msg(Message& msg_, call_back _cb) = 0;
		virtual void send_msg(Message& msg_) = 0;

		virtual void send(Message& msg, uint16_t& _session_id) = 0;
		
		virtual void Response(Message& msg) = 0;

		virtual bool method_call_exec(uint16_t _service_id, uint16_t _instance_id, Message& msg_) = 0;
		//client if

		virtual call_back_id subscribe(uint16_t _service_id, uint16_t _instance_id, uint16_t _evend_id, call_back _cb, uint16_t _eventgroup_id = 0x4465, major_t major = kt_someip::ANY_MAJOR, bool is_field = false) = 0;

		virtual void unsubscribe(uint16_t _service_id, uint16_t _instance_id, uint16_t _evend_id, call_back_id _cb_id, uint16_t _eventgroup_id = 0x4465, major_t _major_id = kt_someip::ANY_MAJOR) = 0;


		virtual void event_get(uint16_t _proxy_id, Message& msg) = 0;

		virtual bool is_event_availabe(uint16_t _proxy_id, uint16_t _service_id, uint16_t _instance_id, uint16_t _event_id) = 0;

		//server if
		virtual void offer_event(uint16_t _service_id, uint16_t _instance_id, uint16_t _event_id, uint16_t _eventgroup_id = 0x4465, major_t major = kt_someip::ANY_MAJOR, bool is_field = false) = 0;

		virtual void stop_offer_event(uint16_t _service_id, uint16_t _instance_id, uint16_t _event_id, major_t major = kt_someip::ANY_MAJOR) = 0;
		
		virtual void notify(Message& _msg) = 0;

		virtual void insert_request_service_info(uint16_t _service_id, uint16_t _instance_id, major_t major = kt_someip::ANY_MAJOR, client_id_t _clientId = 1) = 0;
		
		virtual void registerMethodCallBack(uint16_t _service_id, call_back _call_back) = 0;

		virtual void registerServiceStatusCallBack(uint16_t _service_id, uint16_t _instance_id, major_t _major_id,ServiceStatusCallback _cb) = 0;
		virtual void unregisterServiceStatusCallBack(uint16_t _service_id) = 0;
		//身份验证
		virtual bool is_has_auth(service_t m, client_t c) = 0;
		
		//身份验证
		virtual bool is_has_auth(std::string& m, std::string& c) = 0;

#ifdef CONFIG_ENABLE_PHM
		// 发送异常信息
		virtual bool send_except_use_phm(ara::phm::ENUM_SUBSYSTEM_TYPE subSysType, 
									ara::phm::ENUM_PLATFORM_APP_TYPE platAppType, 
									ara::phm::STATUS_EXCEPTION_TYPE stsExceptType,
									ara::phm::ENUM_DOMAIN_INFORMATION_TYPE domainInfoType, 
									uint16_t serId,
									uint16_t methodId,
									std::string faultDetail,
									uint8_t type) = 0;
		// 发送调用链
		virtual bool send_call_chain_use_phm(long tsHigh, long tsLow, 
										std::string messageId, 
										std::string clientId, 
										int32_t sendType, 
										int32_t executionState,
										uint8_t type) = 0;
#endif // CONFIG_ENABLE_PHM

	};
	
};// namespace ara_runtime



#endif // ARA_RUNTIME_HPP
