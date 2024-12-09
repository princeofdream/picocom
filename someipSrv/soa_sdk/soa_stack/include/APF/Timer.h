#ifndef _APF_TIMER_H_
#define _APF_TIMER_H_
#include <thread>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <mutex>
#include <asio.hpp>
namespace apf {
    namespace utility {
        class TimerContextInstance {
        private:
            TimerContextInstance()
            {
                m_context.stop();
            }
        public:
            ~TimerContextInstance()
            {
                if (!m_context.stopped())
                {
                    m_context.stop();
                    std::unique_lock<std::mutex> guard(m_exit_mutex);
                    m_cv.wait_for(guard, std::chrono::milliseconds(100));
                }
            }
            static TimerContextInstance& GetInstance()
            {
                static TimerContextInstance s_tci;
                return s_tci;
            }
            void run() {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_context.stopped())
                {
                    m_context.reset();
                    std::thread([&]() {
#ifndef _WIN32
                        pthread_setname_np(pthread_self(), "TimerMainContext");
#endif
                        m_context.run();
                        std::unique_lock<std::mutex> guard(m_exit_mutex);
                        m_cv.notify_one();
                    }).detach();
                }
            }
            asio::io_context& GetContext()
            {
                return m_context;
            }
        private:
            std::mutex m_mutex;
            asio::io_context m_context;
            std::mutex m_exit_mutex;
            std::condition_variable m_cv;
        };

        class TimerInternal : public std::enable_shared_from_this<TimerInternal> {
        public:
            TimerInternal() :m_timer(TimerContextInstance::GetInstance().GetContext()), m_interval(0), m_timeout(nullptr), m_running(false)
            {}
            void start(const std::chrono::milliseconds& interval, const std::function<void(void)>& timeout)
            {
                stop();
                std::lock_guard<std::mutex> lock(m_mutex);
                m_interval = interval;
                m_timeout = timeout;
                m_timer.expires_from_now(interval);
                m_timer.async_wait([ptr = shared_from_this()](const asio::error_code& error) {
                    ptr->callback(error);
                });
                m_running = true;
                TimerContextInstance::GetInstance().run();
            }
            void startOneShot(const std::chrono::milliseconds& interval, const std::function<void(void)>& timeout)
            {
                stop();
                std::lock_guard<std::mutex> lock(m_mutex);
                m_interval = interval;
                m_timeout = timeout;
                m_timer.expires_from_now(interval);
                m_timer.async_wait([ptr = shared_from_this()](const asio::error_code& error) {
                    ptr->callback_once(error);
                });
                m_running = true;
                TimerContextInstance::GetInstance().run();
            }
            void stop()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_timer.cancel();
                m_running = false;
                m_timeout = nullptr;
            }

            bool isActive()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_running;
            }
        private:
            void callback(const asio::error_code& error)
            {
                if (!error && m_timeout != nullptr)
                {
                    m_timer.expires_from_now(m_interval);
                    m_timer.async_wait([ptr = shared_from_this()](const asio::error_code& error) {
                        ptr->callback(error);
                    });
                    TimerContextInstance::GetInstance().run();
                    m_timeout();
                }
                else if (error.value() == asio::error::operation_aborted)
                {
                    //only cancel one timer will reach here, I will do nothing, since I want to ignore this event.
                }
                else
                {
                    //I donot know which case will enter here
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_running = false;
                    m_timeout = nullptr;
                }
            }

            void callback_once(const asio::error_code& error)
            {
                std::function<void(void)> callback = nullptr;
				if(error.value() != asio::error::operation_aborted)
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_running = false;
                    callback = m_timeout;
                    m_timeout = nullptr;
                }
                if (!error && callback != nullptr)
                {
                    callback();
                }


            }
        private:
            std::mutex m_mutex;
            asio::steady_timer m_timer;
            std::chrono::milliseconds m_interval;
            std::function<void(void)> m_timeout;
            bool m_running;
        };

        class Timer final
        {
        public:
            Timer() {
                m_timer_internal = std::make_shared<TimerInternal>();
            }
            Timer(Timer&&) = delete;
            Timer& operator=(Timer&&) = delete;

            Timer(const Timer&) = delete;
            Timer& operator=(const Timer&) = delete;

            using Interval = std::chrono::milliseconds;
            using Timeout = std::function<void(void)>;
            using Clock = std::chrono::steady_clock;
            using TimeoutReturnsNextInterval = std::function<Interval(void)>;

            ~Timer()
            {
                stop();
            }

            void start(const Interval& interval, const Timeout& timeout, std::string _name = "TIMER")
            {
                m_timer_internal->start(interval, timeout);
            }

            void startOneShot(const Interval& interval, const Timeout& timeout, std::string _name = "1TIMER")
            {
                m_timer_internal->startOneShot(interval, timeout);
            }

            void stop()
            {
                m_timer_internal->stop();
            }

            bool isActive()
            {
                return m_timer_internal->isActive();
            }

        private:
            std::shared_ptr<TimerInternal> m_timer_internal;
        };


    }
}

#endif
