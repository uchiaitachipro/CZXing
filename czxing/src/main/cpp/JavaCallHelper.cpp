//
// Created by Administrator on 2019/08/10 0010.
//

#include "JavaCallHelper.h"
#include "JNIUtils.h"

JavaCallHelper::JavaCallHelper(JavaVM *_javaVM, JNIEnv *_env, jobject &_jobj) : javaVM(_javaVM),
                                                                                env(_env) {
    // 获取Class
//    jclass jSdkClass = env->FindClass("me/devilsen/czxing/code/NativeSdk");
//    if (jSdkClass == nullptr) {
//        LOGE("Unable to find class");
//        return;
//    }
//
//    // 获取构造函数
//    jmethodID constructor = env->GetMethodID(jSdkClass, "<init>", "()V");
//    if (constructor == nullptr) {
//        LOGE("can't constructor jClass");
//        return;
//    }

    // 获取对应函数
//    jSdkObject = env->NewObject(jSdkClass, constructor);
//    if (jSdkObject == nullptr) {
//        LOGE("can't new jobject");
//        return;
//    }
    jSdkObject = env->NewGlobalRef(_jobj);

    jclass jSdkClass = env->GetObjectClass(jSdkObject);
    if (jSdkClass == nullptr) {
        LOGE("Unable to find class");
        return;
    }

    jmid_on_result = env->GetMethodID(jSdkClass, "onDecodeCallback", "(Ljava/lang/String;DI[FLme/devilsen/czxing/thread/FrameData;)V");
    jmid_on_brightness = env->GetMethodID(jSdkClass, "onBrightnessCallback", "(Z)V");
    jmid_on_collect_performance_data = env->GetMethodID(jSdkClass,"onCollectPerformanceData","(Ljava/lang/String;)V");
    if (jmid_on_result == nullptr) {
        LOGE("jmid_on_result is null");
    }
}

JavaCallHelper::~JavaCallHelper() {
    env->DeleteGlobalRef(jSdkObject);
    DELETE(javaVM);
    DELETE(env);
}

void JavaCallHelper::onResult(const FrameData &frameData,const ZXing::Result &result,double cameraLight = 0) {
//    if (result.format() == ZXing::BarcodeFormat::QR_CODE) {
//        if (result.resultPoints().size() < 2) {
//            return;
//        }
//    } else if (!result.isValid()) {
//        return;
//    }

    if (!result.isValid() && !result.isBlurry()) {
        return;
    }

    int getEnvStat = javaVM->GetEnv((void **) &env, JNI_VERSION_1_6);
    int mNeedDetach = JNI_FALSE;
    if (getEnvStat == JNI_EDETACHED) {
        //如果没有， 主动附加到jvm环境中，获取到env
        if (javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return;
        }
        mNeedDetach = JNI_TRUE;
    }

    std::string contentWString;
    jstring mJstring = nullptr;
    jint format = -1;
    jfloatArray pointsArray = env->NewFloatArray(0);

    if (result.isValid() && !result.text().empty()) {
        contentWString = UnicodeToANSI(result.text());
        mJstring = env->NewStringUTF(contentWString.c_str());
        format = static_cast<int>(result.format());
    }

    if (result.isBlurry()) {
        std::vector<ZXing::ResultPoint> resultPoints = result.resultPoints();
        int size = static_cast<int>(result.resultPoints().size() * 2);
        pointsArray = env->NewFloatArray(size);

        jfloat points[size];

        int index = 0;
        for (auto point : resultPoints) {
            points[index++] = point.x();
            points[index++] = point.y();
        }

        env->SetFloatArrayRegion(pointsArray, 0, size, points);
    }


    auto frameDataClazz = env->FindClass("me/devilsen/czxing/thread/FrameData");
    auto ctor = env->GetMethodID(frameDataClazz, "<init>", "()V");
    auto obj = env->NewObject(frameDataClazz,ctor);

    auto leftField = env->GetFieldID(frameDataClazz,"left","I");
    env->SetIntField(obj,leftField,frameData.left);

    auto topField = env->GetFieldID(frameDataClazz,"top","I");
    env->SetIntField(obj,topField,frameData.top);

    auto widthField = env->GetFieldID(frameDataClazz,"width","I");
    env->SetIntField(obj,widthField,frameData.cropWidth);

    auto heightField = env->GetFieldID(frameDataClazz,"height","I");
    env->SetIntField(obj,heightField,frameData.cropHeight);

    auto rowWidthField = env->GetFieldID(frameDataClazz,"rowWidth","I");
    env->SetIntField(obj,rowWidthField,frameData.rowWidth);

    auto rowHeightField = env->GetFieldID(frameDataClazz,"rowHeight","I");
    env->SetIntField(obj,rowHeightField,frameData.rowHeight);

    auto getDataMethodId = env->GetMethodID(frameDataClazz,"setData","([B)V");
    env->CallVoidMethod(obj,getDataMethodId,*frameData.rawData);

    env->CallVoidMethod(jSdkObject, jmid_on_result, mJstring,cameraLight, format, pointsArray,obj);

    //释放当前线程
    if (mNeedDetach) {
        javaVM->DetachCurrentThread();
    }

}

void JavaCallHelper::onBrightness(const bool isDark) {
    int getEnvStat = javaVM->GetEnv((void **) &env, JNI_VERSION_1_6);
    int mNeedDetach = JNI_FALSE;
    if (getEnvStat == JNI_EDETACHED) {
        //如果没有， 主动附加到jvm环境中，获取到env
        if (javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return;
        }
        mNeedDetach = JNI_TRUE;
    }

    env->CallVoidMethod(jSdkObject, jmid_on_brightness, isDark);

    //释放当前线程
    if (mNeedDetach) {
        javaVM->DetachCurrentThread();
    }

}

void JavaCallHelper::onCollect(const std::string data) {

    int getEnvStat = javaVM->GetEnv((void **) &env, JNI_VERSION_1_6);
    int mNeedDetach = JNI_FALSE;
    if (getEnvStat == JNI_EDETACHED) {
        //如果没有， 主动附加到jvm环境中，获取到env
        if (javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return;
        }
        mNeedDetach = JNI_TRUE;
    }

    auto c_string = data.c_str();
    //定义java String类 strClass
    jclass strClass = env->FindClass("java/lang/String");
    //获取String(byte[],String)的构造器,用于将本地byte[]数组转换为一个新String
    jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
    //建立byte数组
    jbyteArray bytes = env->NewByteArray(strlen(c_string));
    //将char* 转换为byte数组
    (env)->SetByteArrayRegion(bytes, 0, strlen(c_string), (jbyte*) c_string);
    // 设置String, 保存语言类型,用于byte数组转换至String时的参数
    jstring encoding = env->NewStringUTF("GB2312");
    //将byte数组转换为java String,并输出
    auto result = (jstring) env->NewObject(strClass, ctorID, bytes, encoding);

    env->CallVoidMethod(jSdkObject, jmid_on_collect_performance_data, result);

    //释放当前线程
    if (mNeedDetach) {
        javaVM->DetachCurrentThread();
    }
}
