//
// Created by uchiachen on 2019-09-27.
//

#ifndef CZXING_ANDROID_UTILS_H
#define CZXING_ANDROID_UTILS_H


#include <android/bitmap.h>
#include <opencv2/opencv.hpp>

using namespace cv;

void bitmap_to_mat(JNIEnv *env, jobject &srcBitmap, Mat &srcMat);

void mat_to_bitmap(JNIEnv *env, Mat &srcMat, jobject &dstBitmap);

std::string getTimes();

void writeImage(const Mat& mat,std::string prefix = "");

#endif //CZXING_ANDROID_UTILS_H