#ifndef _ARA_RUNTIME_IMPL_HPP_
#define _ARA_RUNTIME_IMPL_HPP_

#include "ara_runtime.hpp"
#include <thread>
#include <condition_variable>
#include <map>
#include <deque>
#include <queue>
#include <list>

#include "Application.hpp"
#include "plugin_manager_impl.h"
#include "access_control_plugin.h"
#ifdef CONFIG_ENABLE_PHM
#include "phm/phm_message.h"
#include "phm/phm.h"
#endif // CONFIG_ENABLE_PHM

namespace ara {
	enum E_PRIORITY_VALUE
	{
		E_PRIORITY_VALUE_0 = 0,
		E_PRIORITY_VALUE_1,
		E_PRIORITY_VALUE_2,
		E_PRIORITY_VALUE_3
	};

	template<typename T, typename V, typename F>
	class ThreadsafePriorityQueue
	{
	public:
		void push(T t)
		{

			std::lock_guard<std::mutex> lk(queueMutex_);
			queue_.push(t);
			condition_.notify_one();
		}

		void waitAndPop(T& t)
		{
			std::unique_lock<std::mutex> lk(queueMutex_);
			condition_.wait(lk, [this]{ return !queue_.empty();});
			t = queue_.top();
			queue_.pop();
		}
		int32_t getSize()
		{
			return queue_.size();
		
		}
	private:
		std::priority_queue<T, V, F> queue_;
		std::condition_variable condition_;
		std::mutex queueMutex_;
	};

	template<typename T>
	class PriorityData 
	{
	public:
		PriorityData(E_PRIORITY_VALUE p, T t) : ePriority(p), data(t){}

		PriorityData(uint8_t p, T t) : data(t)
		{
			if (p <= 3)
			{
				ePriority = (E_PRIORITY_VALUE)p;
			}
			else
			{
				ePriority = E_PRIORITY_VALUE::E_PRIORITY_VALUE_0;
			}
		}

		PriorityData() = default;

		friend bool operator<(PriorityData a, PriorityData b)
		{
			return a.ePriority < b.ePriority;
		}

		friend bool operator>(PriorityData a, PriorityData b)
		{
			return a.ePriority > b.ePriority;
		}

		const T& getData() const
		{
			return data;
		}

		using CompareFunc = std::function<bool(PriorityData, PriorityData)>;
		static CompareFunc getCompareFunc() 
		{
			return []( PriorityData l, PriorityData r){return l > r;};
		}

		
	private:
		E_PRIORITY_VALUE ePriority;
		T data;
	};

	class Runtime_Impl : public Runtime {
#ifdef KT_UT
	public:
#endif
		using proxy_id_t = uint16_t;
		using event_info = std::pair<Message, std::set<proxy_id_t>>;
		using spReq = std::shared_ptr<kt_someip::AppMsgSomeipSendCmd>;


	public:
		Runtime_Impl();
		virtual ~Runtime_Impl();

		static std::shared_ptr<Runtime> get();


		virtual bool init();

		void offer_service(uint16_t _service_id, uint16_t _instance_id, major_t _major = kt_someip::ANY_MAJOR, minor_t _minor = kt_someip::ANY_MINOR);
		
		void stop_offer_service(uint16_t _service_id, uint16_t _instance_id, major_t _major = kt_someip::ANY_MAJOR, minor_t _minor = kt_someip::ANY_MINOR);

		bool find_service(uint16_t _service_id, uint16_t _instance_id, major_t _major_id = kt_someip::ANY_MAJOR);

		virtual void send_msg(Message &msg_);

		virtual void send_msg(Message& msg_, call_back _cb);

		virtual call_back_id subscribe(uint16_t _service_id, uint16_t _instance_id, uint16_t _evend_id, call_back _cb, uint16_t _eventgroup_id = 0x4465, major_t major = kt_someip::ANY_MAJOR, bool is_field = false);

		virtual void send(Message& msg, uint16_t& _session_id);

		void Response(Message& msg);

		void notify(Message& _msg);

		void subscribe(uint16_t _proxy_id, uint16_t _service_id, uint16_t _instance_id, uint16_t _evend_id, uint16_t _eventgroup_id = 0x4465, major_t major = kt_someip::ANY_MAJOR, bool is_field = false);

		void unsubscribe(uint16_t _service_id, uint16_t _instance_id, uint16_t _evend_id, call_back_id _cb_id, uint16_t _eventgroup_id = 0x4465, major_t _major_id = kt_someip::ANY_MAJOR);

		bool method_call_exec(uint16_t _service_id, uint16_t _instance_id, Message& msg_);

		void offer_event(uint16_t _service_id, uint16_t _instance_id, uint16_t _event_id, uint16_t _eventgroup_id = 0x4465, major_t major = kt_someip::ANY_MAJOR, bool is_field = false);

		void stop_offer_event(uint16_t _service_id, uint16_t _instance_id, uint16_t _event_id, major_t major = kt_someip::ANY_MAJOR);
		
		virtual void event_get(uint16_t _proxy_id, Message& msg);

		bool is_event_availabe(uint16_t _proxy_id, uint16_t _service_id, uint16_t _instance_id, uint16_t _event_id);

		void insert_request_service_info(uint16_t _service_id, uint16_t _instance_id, major_t major = kt_someip::ANY_MAJOR, client_id_t _clientId = 1);

		bool is_has_auth(service_t m, client_t c);

		bool is_has_auth(std::string& m, std::string& c);

#ifdef CONFIG_ENABLE_PHM
		bool send_except_use_phm(ara::phm::ENUM_SUBSYSTEM_TYPE subSysType, 
									ara::phm::ENUM_PLATFORM_APP_TYPE platAppType, 
									ara::phm::STATUS_EXCEPTION_TYPE stsExceptType,
									ara::phm::ENUM_DOMAIN_INFORMATION_TYPE domainInfoType, 
									uint16_t serId,
									uint16_t methodId,
									std::string faultDetail,
									uint8_t type);

		bool send_call_chain_use_phm(long tsHigh, long tsLow, 
										std::string messageId, 
										std::string clientId, 
										int32_t sendType, 
										int32_t executionState,
										uint8_t type);
#endif // CONFIG_ENABLE_PHM

#ifdef UNIT_TEST
		std::map< event_t, std::set<proxy_id_t>>& get_event_reg() {
			return m_event_reg;
		}

		std::deque<event_info>& get_receive_msg() {
			return m_event_receive;
		}
#endif

		void registerMethodCallBack(uint16_t _service_id, call_back _call_back);

		void registerServiceStatusCallBack(uint16_t _service_id, uint16_t _instance_id, major_t _major_id,ServiceStatusCallback _cb);
		void unregisterServiceStatusCallBack(uint16_t _service_id);
#ifndef KT_UT
	private:
#endif
		void on_message(const std::shared_ptr<kt_someip::AppMsgSomeipSendCmd>& _request);

		void on_state(kt_someip::remote_state_type_e _state);

		void start();

		void on_availability(service_t _service, instance_t _instance, major_t _major ,minor_t _minor, bool _is_available);

		void on_subscribe_event_avail(const service_t s, const instance_t t,
									const major_t m, const kt_someip::AppMsgEventAckCmd::e_evt_ack_type_t type, eventgroup_t egrp,
									const std::set<event_t>& events, uint64_t ts);

		void on_err_major_ver_handler(const service_t s, const instance_t t,
									const major_t cm, const major_t em, const kt_someip::ExpMajorVersionCmd::err_type type,
									const uint16_t rmt_port, const std::string rmt_ip, uint64_t ts, uint16_t midOrEid);

#ifdef CONFIG_ENABLE_PHM
		inline void process_sub_problem_to_phm(const service_t s, const instance_t t,
									const major_t m, const kt_someip::AppMsgEventAckCmd::e_evt_ack_type_t type, eventgroup_t egrp,
									const std::set<event_t>& events, uint64_t ts);

		inline void process_ver_problem_to_phm(const service_t s, const instance_t t,
									const major_t cm, const major_t em, const kt_someip::ExpMajorVersionCmd::err_type type,
									const uint16_t rmt_port, const std::string rmt_ip, uint64_t ts, uint16_t midOrEid);
#endif // CONFIG_ENABLE_PHM

		bool is_method_messge(apf::someip::MessageStandardHeader::MessageType _type);

		bool is_event_message(apf::someip::MessageStandardHeader::MessageType _type);

		void do_loop_process_queue();
		void process_req(spReq& _spreq);
#ifndef KT_UT
	private:
#endif
		void notificationHandle(const std::shared_ptr<kt_someip::AppMsgSomeipSendCmd>& _notify);
		bool init_flag;
		kt_someip::Application* m_app;
		kt_someip::remote_state_type_e m_register_sts;
		std::thread start_;
		std::condition_variable register_condition_;
		std::mutex mutex_;
		bool isStateOk_;
#ifdef CONFIG_ENABLE_PHM
		std::mutex mutex_for_phm_;
#endif // CONFIG_ENABLE_PHM
		bool register_block;
		std::deque<Message> m_message_receive;
		std::mutex m_message_receive_mutex;
		std::map<service_t, std::map<instance_t, std::map<major_t,bool>>> m_offered_service;
		std::mutex m_offered_service_mutex;
		std::deque<kt_someip::_service_data> m_requested_service;
		std::mutex m_requested_service_mutex;
#ifdef UNIT_TEST
		std::map< event_t, std::set<proxy_id_t>> m_event_reg;
#endif
		std::deque<event_info> m_event_receive;
		std::mutex m_event_receive_mutex;

		std::map<session_id_t, call_back> m_method_cb;
		std::mutex m_method_cb_mutex;
		std::map<std::string, std::list<std::pair<call_back_id,call_back>> > m_event_cb;
		std::mutex m_event_cb_mutex;

		client_id_t m_clientId;
		major_t m_majorVersion;
		std::map<service_t, call_back> m_call_back;
		std::mutex m_call_back_mutex;
		std::map<service_t, ServiceStatusCallback> cbServiceStatus_;
		std::mutex cbServiceStatus_mutex;
		PluginManager* m_pm;
		std::shared_ptr<AccessControlPlugin> m_access_ctl;
#ifdef CONFIG_ENABLE_PHM
		ara::phm::Phm phm_;
#endif //CONFIG_ENABLE_PHM
		std::thread thProcessReq_;
		std::mutex queueMutex_;
		std::condition_variable condition_;
		std::priority_queue<PriorityData<spReq>, 
                        std::vector<PriorityData<spReq>>, 
                        PriorityData<spReq>::CompareFunc> reqPriQueue_;
		ThreadsafePriorityQueue<PriorityData<spReq>, 
                        std::vector<PriorityData<spReq>>, 
                        PriorityData<spReq>::CompareFunc> reqPriQueueSafe_;
	};

}

#endif
