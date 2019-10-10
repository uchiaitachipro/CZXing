//
// Created by uchiachen on 2019-09-27.
//

#include "android_utils.h"
#include <time.h>
#include <chrono>
#include <sstream>

void bitmap_to_mat(JNIEnv *env, jobject &srcBitmap, Mat &srcMat) {
    void *srcPixels = 0;
    AndroidBitmapInfo srcBitmapInfo;
    try {
        AndroidBitmap_getInfo(env, srcBitmap, &srcBitmapInfo);
        AndroidBitmap_lockPixels(env, srcBitmap, &srcPixels);
        uint32_t srcHeight = srcBitmapInfo.height;
        uint32_t srcWidth = srcBitmapInfo.width;
        srcMat.create(srcHeight, srcWidth, CV_8UC4);
        if (srcBitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            Mat tmp(srcHeight, srcWidth, CV_8UC4, srcPixels);
            tmp.copyTo(srcMat);
        } else {
            Mat tmp = Mat(srcHeight, srcWidth, CV_8UC2, srcPixels);
            cvtColor(tmp, srcMat, COLOR_BGR5652RGBA);
        }
        AndroidBitmap_unlockPixels(env, srcBitmap);
        return;
    } catch (cv::Exception &e) {
        AndroidBitmap_unlockPixels(env, srcBitmap);
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return;
    } catch (...) {
        AndroidBitmap_unlockPixels(env, srcBitmap);
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "unknown");
        return;
    }
}

void mat_to_bitmap(JNIEnv *env, Mat &srcMat, jobject &dstBitmap) {
    void *dstPixels = 0;
    AndroidBitmapInfo dstBitmapInfo;
    try {
        AndroidBitmap_getInfo(env, dstBitmap, &dstBitmapInfo);
        AndroidBitmap_lockPixels(env, dstBitmap, &dstPixels);
        uint32_t dstHeight = dstBitmapInfo.height;
        uint32_t dstWidth = dstBitmapInfo.width;
        if (dstBitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
            Mat tmp(dstHeight, dstWidth, CV_8UC4, dstPixels);
            if (srcMat.type() == CV_8UC1) {
                cvtColor(srcMat, tmp, COLOR_GRAY2RGBA);
            } else if (srcMat.type() == CV_8UC3) {
                cvtColor(srcMat, tmp, COLOR_RGB2RGBA);
            } else if (srcMat.type() == CV_8UC4) {
                srcMat.copyTo(tmp);
            }
        } else {
            Mat tmp = Mat(dstHeight, dstWidth, CV_8UC2, dstPixels);
            if (srcMat.type() == CV_8UC1) {
                cvtColor(srcMat, tmp, COLOR_GRAY2BGR565);
            } else if (srcMat.type() == CV_8UC3) {
                cvtColor(srcMat, tmp, COLOR_RGB2BGR565);
            } else if (srcMat.type() == CV_8UC4) {
                cvtColor(srcMat, tmp, COLOR_RGBA2BGR565);
            }
        }
        AndroidBitmap_unlockPixels(env, dstBitmap);
    } catch (cv::Exception &e) {
        AndroidBitmap_unlockPixels(env, dstBitmap);
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, e.what());
        return;
    } catch (...) {
        AndroidBitmap_unlockPixels(env, dstBitmap);
        jclass je = env->FindClass("java/lang/Exception");
        env->ThrowNew(je, "unknown");
        return;
    }
}


std::string getTimes() {
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp =
            std::chrono::time_point_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now());//获取当前时间点
    std::time_t timestamp = tp.time_since_epoch().count(); //计算距离1970-1-1,00:00的时间长度

    std::string number;
    std::stringstream strstream;
    strstream << timestamp;
    strstream >> number;
    return number;
}


long flag = 0;
void writeImage(const Mat& mat,std::string prefix){
    Mat result;
    mat.copyTo(result);
    std::string str;
    str.append("/storage/emulated/0/scan/");
    str.append(prefix);
    str.append(getTimes());
    str.append(std::to_string(flag++));
    str.append(".jpg");
    imwrite(str, result);
}
