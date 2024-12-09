#ifdef __ANDROID__ 
#include "android/log.h"
#include "log.hpp"
namespace ServiceCallBack {
    class JniCallBack {
    public:
        JavaVM *javaVM;
        jobject serviceCallBack = NULL;
        jobject serviceResult = NULL;

        jstring cstringToJstring(JNIEnv *env, std::string str) {    
            int len = str.length();
            char *data = (char *) malloc((len + 1) * sizeof(char));   
            data[len] = 0;
            str.copy(data, static_cast<unsigned int>(len), 0);
            jclass strClass = env->FindClass("java/lang/String");
            jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
            jbyteArray bytes = env->NewByteArray(len);
            env->SetByteArrayRegion(bytes, 0, len, (jbyte *) data);
            jstring encoding = env->NewStringUTF("utf-8");
            free(data);
            jstring result = static_cast<jstring>(env->NewObject(strClass, ctorID, bytes, encoding));
            env->DeleteLocalRef(strClass);    
            return result;
        }

        void onMessageReceived(service_t service_id, instance_t instance_id, major_t majorVersion, bool _is_available) {
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
            if (serviceCallBack != NULL && serviceResult != NULL) {
                LOGD("serviceCallBack&&serviceResult!=null");
            
                jclass cls = env->GetObjectClass(serviceCallBack);
                jmethodID methodId = env->GetMethodID(cls, "onStatus",
                                                      "(Lcom/kotei/ktsomeip/ServiceStatus;)V");

                    jclass result_class = env->GetObjectClass(serviceResult);

                    jmethodID jinstance_id = env->GetMethodID(result_class, "setInstanceId", "(Ljava/lang/String;)V");
                    jmethodID jservice_status = env->GetMethodID(result_class, "setStatus",
                                                          "(Z)V");
                    env->CallVoidMethod(serviceResult, jinstance_id, cstringToJstring(env,kt_someip::utils::dec2HexStr(instance_id)));
                    LOGD("service_id:%s,serviceStatus:%s",kt_someip::utils::dec2HexStr(service_id).c_str(),std::to_string(_is_available).c_str());

                    env->CallVoidMethod(serviceResult, jservice_status,  (jboolean)_is_available);

                    env->CallVoidMethod(serviceCallBack, methodId, serviceResult);
                    env->DeleteLocalRef(result_class);
                    env->DeleteLocalRef(cls);
            } else {
                LOGE("serviceCallBack||serviceResult == null!");
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
            if (serviceResult != NULL) {
                env->DeleteGlobalRef(serviceResult);
                serviceResult = NULL;
            } else if (serviceCallBack != NULL) {
                env->DeleteGlobalRef(serviceCallBack);
                serviceCallBack = NULL;
            }
        }
    };

}
#endif