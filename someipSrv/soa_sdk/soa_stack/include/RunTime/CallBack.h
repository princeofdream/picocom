#ifdef __ANDROID__ 
#include "android/log.h"
#define LOG "CommunicationJNI" //TAG
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG,__VA_ARGS__)
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG,__VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG,__VA_ARGS__)
#include "../include/ara_message.hpp"

namespace CallBack {
        static const std::string RETURN_OK="000";

        static const std::string DESERIALIZE_ERROR="001";

        static const std::string NOMETHODID_ERROR="003";
    class JniCallBack {
    public:
        JavaVM *javaVM;
        jobject methodCallBack = NULL;
        jobject methodResult = NULL;
        ara::Message requestMsg;  

        void onMessageReceived(ara::Message msg) {
            JNIEnv *env = NULL;
            bool hasAttach = false;
            if (javaVM == NULL) {
                LOGE("javaVM  null!");
                return;
            }
            jint ret = javaVM->GetEnv((void **) &env, JNI_VERSION_1_6);
            if (ret < 0) {
                jint status = javaVM->AttachCurrentThread(&env, NULL);
                if (status < 0) {
                    return;
                } else {
                    hasAttach = true;
                }
            }
            if (methodCallBack != NULL && methodResult != NULL) {
                // LOGD("methodCallBack&&methodResult!=null");
            
                jclass cls = env->GetObjectClass(methodCallBack);
                jmethodID methodId = env->GetMethodID(cls, "onMessageReceive",
                                                      "(Lcom/kotei/ktsomeip/Result;)V");
                apf::someip::MessageStandardHeader::ReturnCode returnCode = msg.getMessage()->getMessage()->getMessageHeader().getReturnCode();

    
                requestMsg = msg;
                jclass result_class = env->GetObjectClass(methodResult);
// 获取 jmethodID  (GetMethodID:第一个参数：jclass 第二个参数：要调用的方法名
// 第三个参数 "(Ljava/lang/String;)V" 传入 java/lang/String 参数 返回Void
// V -> void （L ；）引用类型写法 如果是Int 可以写成 (I)V  也可以写成  "(Ljava/lang/Integer;)V"
                jmethodID jsession = env->GetMethodID(result_class, "setSession_id", "(I)V");
                jmethodID jmethod = env->GetMethodID(result_class, "setId", "(I)V");
                jmethodID jclient_id = env->GetMethodID(result_class, "setClient_id", "(I)V");
                jmethodID jmessage = env->GetMethodID(result_class, "setContent",
                                                          "([B)V");
                jmethodID jreturnCode = env->GetMethodID(result_class, "setReturnCode",
                                                          "(Ljava/lang/String;)V");
                env->CallVoidMethod(methodResult, jsession, msg.getSession());
                env->CallVoidMethod(methodResult, jmethod, msg.getMethod());
                env->CallVoidMethod(methodResult, jclient_id, msg.getClientId());
     
                // LOGD("callback onMessageReceive returnCode:%d",returnCode);

                if (returnCode == apf::someip::MessageStandardHeader::ReturnCode::E_OK)
                {
                    env->CallVoidMethod(methodResult, jreturnCode, env->NewStringUTF(RETURN_OK.c_str()));

                }else if (returnCode == apf::someip::MessageStandardHeader::ReturnCode::E_MALFORMED_MESSAGE)
                {
                    env->CallVoidMethod(methodResult, jreturnCode, env->NewStringUTF(DESERIALIZE_ERROR.c_str()));

                }else if (returnCode == apf::someip::MessageStandardHeader::ReturnCode::E_UNKNOWN_METHOD)
                {
                    env->CallVoidMethod(methodResult, jreturnCode, env->NewStringUTF(NOMETHODID_ERROR.c_str()));

                }else {
                    std::string ret_str = "error :" + std::to_string((int)returnCode);
                    LOGE("JNI Callback %s", ret_str.c_str());
                    env->CallVoidMethod(methodResult, jreturnCode, env->NewStringUTF(ret_str.c_str()));
                }
                    
                
                jbyteArray jbytes = env->NewByteArray(msg.getBodyLength());
                env->SetByteArrayRegion(jbytes, 0, msg.getBodyLength(),
                                            (jbyte *) msg.getBodyData());
                env->CallVoidMethod(methodResult, jmessage, jbytes);
                env->CallVoidMethod(methodCallBack, methodId, methodResult);
                env->DeleteLocalRef(jbytes);
                env->DeleteLocalRef(result_class);
                env->DeleteLocalRef(cls);
            } else {
                LOGE("methodCallBack||methodResult == null!");
            }
            if (env->ExceptionCheck()) {
                env->ExceptionDescribe(); // writes to logcat
                env->ExceptionClear();
            }
            if (hasAttach) {
                javaVM->DetachCurrentThread();
            }

        }

        ~JniCallBack() {
            LOGE("JniCallBack destroy!");
            JNIEnv *env = NULL;
            if (javaVM == NULL) {
                LOGE( "javaVM  null!");
                return;
            }
            jint ret = javaVM->GetEnv((void **) &env, JNI_VERSION_1_6);
            if (ret < 0) {
                jint status = javaVM->AttachCurrentThread(&env, NULL);
                if (status < 0) {
                    return;
                }
            }
            if (methodResult != NULL) {
                env->DeleteGlobalRef(methodResult);
                methodResult = NULL;
            } else if (methodCallBack != NULL) {
                env->DeleteGlobalRef(methodCallBack);
                methodCallBack = NULL;
            } else if (requestMsg.getMessage()!=nullptr){
                requestMsg = ara::Message();
            }
        }
    };

}
#endif
