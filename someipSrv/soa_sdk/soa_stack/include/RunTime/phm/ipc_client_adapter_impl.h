#ifndef _IPC_CLIENT_ADAPTER_IMPL_H
#define _IPC_CLIENT_ADAPTER_IMPL_H

#include <string>
#include <iostream>
#include <sys/time.h>

#include "ipc_client_adapter_interface.h"

namespace ara{
namespace phm{

class IPCClientAdapterImpl : public IPCClientAdapterInterface
{
    public:
        IPCClientAdapterImpl();
        ~IPCClientAdapterImpl();

        bool CheckConnection();
        void Connect(std::string& serverIPCPath, timeval& sendTimeout,timeval& recvTimeout, int type);
        void Disconnect();
        ssize_t Receive(void* buffer, std::size_t buffer_size);
        void Send(void* message, std::size_t length);
        // void SendCredentials();
    private:
        int socketDescriptor_;
        std::string m_client_path;
        void AdjustSocketTimeouts(timeval& sendTimeout,timeval& recvTimeout);
        void ConnectSocketToServer(std::string& serverIPCPath);
};

}//phm
}//ara

#endif /*_IPC_CLIENT_ADAPTER_IMPL_H */