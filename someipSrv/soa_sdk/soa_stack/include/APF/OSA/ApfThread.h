
#ifndef _APF_THREAD_H_
#define _APF_THREAD_H_
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <variant>

#include "ApfMessage.h"


template <typename Msg>
class ApfThread {
public:
    ApfThread() {
        //m_thread = std::thread([this] {run();});
    }
   
    void run() {
        initializeBeforeMessageLoop();
        while (1) {
            auto msg = m_message_queue.recievdMessage();
            onMessage(*msg);
        }
    }

    apf::osa::MessageSender<Msg> getMessageSender() {
        return apf::osa::MessageSender<Msg>(m_message_queue);
    }

    void join() {
        m_thread.join();
    }

protected:

    virtual void initializeBeforeMessageLoop() {}
    virtual void onMessage(Msg const& _msg) = 0;

private:
    std::thread       m_thread;
    apf::osa::MessageQueue<Msg> m_message_queue;
};

#endif