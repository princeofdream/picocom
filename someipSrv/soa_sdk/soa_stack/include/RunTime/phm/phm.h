#ifndef _ARA_PHM_H
#define _ARA_PHM_H

#include <memory>
// #include "phm_serializer.h"
#include "phm_message.h"
#include "ipc_parameters.h"
#include "ipc_client_adapter_interface.h"

#define GRANT_SND_BUFF_LEN IPC_MSG_BUFF_LEN
#define GRANT_RCV_BUFF_LEN IPC_MSG_BUFF_LEN

namespace ara
{
    namespace phm
    {
        using namespace apf;
        class Phm
        {
        public:
            Phm();
            ~Phm();
            static uint32_t setFaultCode(
                ENUM_SUBSYSTEM_TYPE subSystemType,
                ENUM_PLATFORM_APP_TYPE platformAppType,
                STATUS_EXCEPTION_TYPE statusExpType,
                ENUM_DOMAIN_INFORMATION_TYPE domainInfoType);
            void reportExceptionMsg(MessageException msg);
            void reportCallChain(MessageCallChain Msg);

        private:
            std::unique_ptr<ara::phm::IPCClientAdapterInterface> ipcClientAdapter;
        };

    } // phm
} // ara
#endif