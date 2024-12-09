#ifndef ARA_RUNTIME_HPP
#define ARA_RUNTIME_HPP
#include<memory>
#include <string.h>

#include <functional>
#include "ara_message.hpp"
#include<functional>
//#include <vsomeip/export.hpp>
#if _WIN32
#define VSOMEIP_EXPORT __declspec(dllexport)
#define VSOMEIP_EXPORT_CLASS_EXPLICIT

#if VSOMEIP_DLL_COMPILATION
#define VSOMEIP_IMPORT_EXPORT __declspec(dllexport)
#else
#define VSOMEIP_IMPORT_EXPORT __declspec(dllimport)
#endif

#if VSOMEIP_DLL_COMPILATION_CONFIG
#define VSOMEIP_IMPORT_EXPORT_CONFIG __declspec(dllexport)
#else
#define VSOMEIP_IMPORT_EXPORT_CONFIG __declspec(dllimport)
#endif
#else
#define VSOMEIP_EXPORT
#define VSOMEIP_IMPORT_EXPORT
#define VSOMEIP_IMPORT_EXPORT_CONFIG
#endif

namespace ara {
	template <typename... T>

	void ignore_unused(T const& ...) {}
	typedef std::function<void (Message) > call_back;

	class VSOMEIP_EXPORT Runtime {
	public:
		virtual ~Runtime() {}
		static std::shared_ptr<Runtime> getInstance();

		virtual bool init() = 0;

		virtual void offer_service(uint16_t _service_id, uint16_t _instance_id) = 0;
		virtual bool find_service(uint16_t _service_id, uint16_t _instance_id) = 0;

		virtual void send_msg(Message& msg_, call_back _cb) = 0;
		virtual void send_msg(Message& msg_) = 0;


		virtual void send(Message& msg, uint16_t& _session_id) = 0;
		
		virtual void Response(Message& msg) = 0;

		virtual bool method_call_exec(uint16_t _service_id, uint16_t _instance_id, Message& msg_) = 0;
		//client if

		virtual call_back_id subscribe(uint16_t _service_id, uint16_t _instance_id, uint16_t _evend_id, call_back _cb) = 0;

		virtual void unsubscribe(uint16_t _service_id, uint16_t _instance_id, uint16_t _evend_id, call_back_id _cb_id) = 0;


		virtual void event_get(uint16_t _proxy_id, Message& msg) = 0;

		virtual bool is_event_availabe(uint16_t _proxy_id, uint16_t _service_id, uint16_t _instance_id, uint16_t _event_id) = 0;

		//server if
		virtual void offer_event(uint16_t _service_id, uint16_t _instance_id, uint16_t _event_id) = 0;

		virtual void notify(Message& _msg) = 0;

		virtual void insert_request_service_info(uint16_t _service_id, uint16_t _instance_id) = 0;

	};
	
};// namespace ara_runtime



#endif // ARA_RUNTIME_HPP
