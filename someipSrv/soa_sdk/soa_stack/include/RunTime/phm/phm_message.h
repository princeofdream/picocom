#ifndef _PHM_MESSAGE_H
#define _PHM_MESSAGE_H
#include <cstdint>
#include <set>
#include "ara/core/string.h"
#include "phm_serializer.h"

namespace ara
{
    namespace phm
    {
        using namespace ara::core;

        enum class ENUM_SUBSYSTEM_TYPE : uint8_t
        {
            TYPE_PLATFORM_APPLICATION       = 0x00, //平台应用或非子系统
            TYPE_VEHICLE_COMMUNICATION      = 0x01, //车机通讯
            TYPE_CABIN_USER_INTERACTION     = 0x02, //座舱用户交互
            TYPE_SMART_DRIVING_INTERACTION  = 0x03, //智驾交互
            TYPE_AUDIO_MANAGEMENT           = 0x04, //音频管理
            TYPE_DISPLAY_MANAGEMENT         = 0x05, //显示管理
            TYPE_ECOLOGICAL_APPLICATION     = 0x06, //生态应用
            TYPE_Telematics                 = 0x07  //Telematics                        
        };

        enum class ENUM_PLATFORM_APP_TYPE : uint8_t
        {
            TYPE_SUBSYSTEM                  = 0x00, //子系统
            TYPE_ELECTRICAL_ARCHITECTURE    = 0x01, //电气架构
            TYPE_VEHICLE_MODE               = 0x02, //车辆模式
            TYPE_NETWORK_COMMUNICATIONS     = 0x03, //网络通信
            TYPE_DIAGNOSTIC_MODULE          = 0x04, //诊断技术
            TYPE_FUNCTIONAL_SAFETY          = 0x05, //功能安全
            TYPE_INFORMATION_SAFETY         = 0x06, //信息安全
            TYPE_BIG_DATA                   = 0x07, //大数据                        
            TYPE_SOA                        = 0x08, //SOA                        
            TYPE_OTA                        = 0x09, //OTA                        
            TYPE_SOFTWARE_PAYMENT           = 0x0A, //软件付费                        
            TYPE_ENGINEERING_MODE           = 0x0B, //工程模式                        
            TYPE_LAST_MODE                  = 0x0C, //LastMode                        
            TYPE_SYSTEM_RESOURCE            = 0x0D  //系统资源                        
        };

        enum class STATUS_EXCEPTION_TYPE : uint16_t
        {
            STATUS_NO_EXP                   = 0x00, //无故障
            STATUS_FUNCTION_LOSS_EXP        = 0x01, //功能丢失异常
            STATUS_SIGNAL_LOSS_EXP          = 0x02, //信号丢失异常
            STATUS_ECU_LOSS_EXP             = 0x03, //ECU丢失异常
            STATUS_SUBSCRIBE_TIMEOUT_EXP    = 0x04, //订阅超时异常
            STATUS_RPC_EXP                  = 0x05, //远程过程调用异常
            STATUS_VERSION_EXP              = 0x06, //版本异常
            STATUS_CYCLE_EVENT_EXP          = 0x07, //周期事件异常
            STATUS_ONCHANGE_EVENT_EXP       = 0x08, //值更新事件异常
            STATUS_SERVICE_PERMISSION_EXP   = 0x09, //请求权限拦截异常
            STATUS_SERVICE_UNAVAILABLE_EXP  = 0x0A  //服务不可用异常
        };

        enum class ENUM_DOMAIN_INFORMATION_TYPE : uint8_t
        {
            TYPE_NO_DOMAIN                  = 0x00, //非域控
            TYPE_CCU                        = 0x01, //CCU
            TYPE_IDC                        = 0x02, //IDC
            TYPE_ADC                        = 0x03, //ADC
            TYPE_Tbox                       = 0x04, //Tbox
        };

        struct MessageException
        {
            friend PhmDeserializer &operator>>(PhmDeserializer &inputBuffer, MessageException &msg)
            {
                inputBuffer >> msg.serviceId;
                inputBuffer >> msg.methodId;
                inputBuffer >> msg.timeStamp;
                inputBuffer >> msg.faultCode;
                inputBuffer >> msg.faultString;
                inputBuffer >> msg.faultReason;
                inputBuffer >> msg.faultDetail;
                inputBuffer >> msg.type;
                return inputBuffer;
            }

            friend PhmSerializer &operator<<(PhmSerializer &outputBuffer, MessageException const &msg)
            {
                outputBuffer << 3;
                outputBuffer << msg.serviceId;
                outputBuffer << msg.methodId;
                outputBuffer << msg.timeStamp;
                outputBuffer << msg.faultCode;
                outputBuffer << msg.faultString;
                outputBuffer << msg.faultReason;
                outputBuffer << msg.faultDetail;
                outputBuffer << msg.type;
                return outputBuffer;
            }

            uint16_t serviceId;
            uint16_t methodId;
            uint64_t timeStamp;
            uint32_t faultCode;
            String faultString = ""; //中文可供阅读的描述信息
            String faultReason = ""; //描述故障产生的原因
            String faultDetail = ""; //对fault_reason的补充，表示故障详细信息
            uint8_t type;
        };

        struct MessageCallChain
        {
            friend PhmDeserializer &operator>>(PhmDeserializer &inputBuffer, MessageCallChain &msg)
            {
                inputBuffer >> msg.eventTimeStamp;
                inputBuffer >> msg.messageId;
                inputBuffer >> msg.clientId;
                inputBuffer >> msg.sendType;
                inputBuffer >> msg.executionState;
                inputBuffer >> msg.type;
                return inputBuffer;
            }

            friend PhmSerializer &operator<<(PhmSerializer &outputBuffer, MessageCallChain const &msg)
            {
                outputBuffer << 4;
                outputBuffer << msg.eventTimeStamp;
                outputBuffer << msg.messageId;
                outputBuffer << msg.clientId;
                outputBuffer << msg.sendType;
                outputBuffer << msg.executionState;
                outputBuffer << msg.type;
                return outputBuffer;
            }

            Array<long,2> eventTimeStamp;
            String messageId;
            String clientId;
            int32_t sendType;
            int32_t executionState;
            uint8_t type;
        };
    } // phm
} // ara

#endif /*_PHM_MESSAGE_H*/