#ifndef _APF_MESSAGE_H_
#define _APF_MESSAGE_H_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <variant>

namespace apf {

namespace osa {


template <typename Msg>
class MessageQueue {

    enum class MsgQueueId {};

public:
    void sendMessage(std::shared_ptr<Msg> _msg) {

        // Add user data msg to queue and notify worker thread
        std::unique_lock<std::mutex> lk(m_mutex);
        m_queue.push(_msg);
        m_cv.notify_one();
    }

    std::shared_ptr<Msg> recievdMessage() {
        // Wait for a message to be added to the queue
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cv.wait(lk, [&]() {return !m_queue.empty(); });
        auto msg = m_queue.front();
        m_queue.pop();

        return msg;
    }

private:
    std::mutex                       m_mutex; 
    std::condition_variable          m_cv;
    std::queue<std::shared_ptr<Msg>> m_queue;
};

template <typename Msg>
class MessageSender {
public:
    MessageSender(MessageQueue<Msg> & _msg_queue) :m_message_queue_proxy(&_msg_queue) {}
    MessageSender() :m_message_queue_proxy(nullptr) {}

    void sendMessage(std::shared_ptr<Msg> _msg) {
        // Add user data msg to queue and notify worker thread
        if (m_message_queue_proxy) {
            m_message_queue_proxy->sendMessage(_msg);
        }
    }

private:
    MessageQueue<Msg> *m_message_queue_proxy;
};


}
}

#endif