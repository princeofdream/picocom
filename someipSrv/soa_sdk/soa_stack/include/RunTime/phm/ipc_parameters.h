#ifndef _IPC_PARAMETERS_H
#define _IPC_PARAMETERS_H

namespace ara
{
    namespace phm
    {
        #define IPC_TRACE_ENABLE 0
        #define SERVER_PATH "/tmp/PHM_IPC_SERVER_PATH"
        #define SEND_TIMEOUT 50
        #define RCV_TIMEOUT 60
        #define SEL_TIMEOUT 70
        #define IPC_MSG_BUFF_LEN 256

    } // phm
} // ara

#endif /*_IPC_PARAMETERS_H */