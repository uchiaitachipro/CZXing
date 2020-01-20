//
// Created by Administrator on 2019/08/10 0010.
//

#ifndef CZXING_JAVACALLHELPER_H
#define CZXING_JAVACALLHELPER_H


#include <jni.h>
#include "Result.h"
#include "FrameData.h"

class JavaCallHelper {
public:
    JavaCallHelper(JavaVM *_javaVM, JNIEnv *_env, jobject &_jobj);

    ~JavaCallHelper();

    void onResult(const FrameData &frameData,const ZXing::Result &result,double cameraLight);

    void onBrightness(const bool isDark);

    void onCollect(const std::string data);

private:
    JavaVM *javaVM;
    JNIEnv *env;
    jobject jSdkObject;
    jmethodID jmid_on_result;
    jmethodID jmid_on_brightness;
    jmethodID jmid_on_collect_performance_data;

};


#endif //CZXING_JAVACALLHELPER_H
