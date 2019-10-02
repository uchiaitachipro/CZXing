//
// Created by uchia on 2019-10-02.
//

#ifndef CZXING_QRCODEFINDER_H
#define CZXING_QRCODEFINDER_H


#include "opencv2/opencv.hpp"
#include <opencv2/imgproc/types_c.h>

using namespace cv;

class QRCodeFinder {
private:
    cv::Mat preProcessMat(const Mat &source,int cannyValue,int blurValue);
    bool checkPositionDetectionPattern(const cv::Mat &source, cv::Rect area);
    cv::Mat rotateArea(cv::Mat &source,cv::RotatedRect area);
public:
    cv::Mat locateQRCode(const Mat &source,int cannyValue,int blurValue,bool isHisEqual);
};


#endif //CZXING_QRCODEFINDER_H
