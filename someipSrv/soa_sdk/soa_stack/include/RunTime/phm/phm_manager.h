
#ifndef _PHM_MANAGER_H_
#define _PHM_MANAGER_H_

#include <sys/select.h>
#include "json.hpp"
#include <list>
#include <mutex>

namespace ara{
namespace phm{
using namespace v1::ara_api::PS_BigData;

// 129对应二进制10000001，前两位10表示QNX,中间两位00是单条消息，后四位0001表示故障消息
typedef struct Type
{
	uint8_t msg_type            :4;         //0000：用户操作数据 0001故障消息，0010调用链数据，0011服务数据
	uint8_t msg_body_type       :2;         //00:单条消息，01：列表
	uint8_t domain              :2;         //00：android端， 01：调用链 ，10：QNX
}Type;

class PhmManager
{

public:
	PhmManager(std::unique_ptr<proxy::PS_BigDataProxy> PS_BigDataProxy);
	~PhmManager();
	void EndEventLoop();
	bool InitServerAdapter();
	bool ConnectServerSocket(ara::core::String& serverIPCPath, timeval& sendTimeout, timeval& recvTimeout, int type, bool block);
	void RunEventLoop();

	void HandleNewClientConnection(int clientSocketDescriptor);
	ssize_t Receive(void* buffer, std::size_t buffer_size, int clientSocketDescriptor);
	ssize_t Send(void* message, std::size_t length, int clientSocketDescriptor);

	void sendMsgToBigDataSkeleton();

private:
	std::list<nlohmann::json> m_msg_call_chain;
	std::list<nlohmann::json> m_msg_exception;
	std::mutex m_msg_mutex;
	std::unique_ptr<proxy::PS_BigDataProxy> data_;
	std::atomic<int> serverSocketDescriptor_;
	int maxfd;
	fd_set rdset;
    fd_set rdtemp;
};

}//phm
}//ara


#endif 
