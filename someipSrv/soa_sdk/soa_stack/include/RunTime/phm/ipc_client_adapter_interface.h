#ifndef _IPC_CLIENT_ADAPTER_INTERFACE_H
#define _IPC_CLIENT_ADAPTER_INTERFACE_H

#include <string>
#include <iostream>
#include <sys/time.h>

namespace ara
{
    namespace phm
    {

        using namespace std;

        class IPCClientAdapterInterface
        {
        public:
            IPCClientAdapterInterface() {}
            ~IPCClientAdapterInterface() {}
            virtual bool CheckConnection() = 0;
            virtual void Connect(std::string &serverIPCPath, timeval &sendTimeout, timeval &recvTimeout, int type) = 0;
            virtual void Disconnect() = 0;
            virtual ssize_t Receive(void *buffer, std::size_t buffer_size) = 0;
            virtual void Send(void *message, std::size_t length) = 0;
            // virtual void SendCredentials() = 0;
        };

    } // phm
} // ara

#endif /*_IPC_CLIENT_ADAPTER_INTERFACE_H */