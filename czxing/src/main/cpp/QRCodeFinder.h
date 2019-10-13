//
// Created by uchia on 2019-10-02.
//

#ifndef CZXING_QRCODEFINDER_H
#define CZXING_QRCODEFINDER_H


#include "opencv2/opencv.hpp"
#include <opencv2/imgproc/types_c.h>

using namespace cv;

struct CandidateRegion;

class QRCodeFinder {
private:
    const float DEVIATION_RATIO = 0.02;
    const float DEVIATION_PIXEL = 5;
    const float EXPAND_RATIO = 0.15;

    float pixel_tolerance = 0.1;
    bool checkPositionDetectionPattern(const cv::Mat &source, cv::RotatedRect area);
    bool check11311Pattern(const cv::Mat &source, LineIterator &l);
    std::vector<std::pair<cv::Point2f,cv::Point2f>> checkTimingPattern(const cv::Mat &source,const RotatedRect &r1, const RotatedRect &r2);
    void filterInscribedRect(std::vector<std::vector<cv::Point>> &source);
    cv::Rect findRegion(std::vector<CandidateRegion> &candidates);
    cv::Rect expandRegion(const cv::Mat &mat,cv::Rect &region);
    bool canTolerate(int basePixel,int currentPixel);
//    cv::Mat rotateArea(const cv::Mat &source,cv::RotatedRect area);
public:
    cv::Mat preProcessMat(const Mat &source,int cannyValue,int blurValue);
    cv::Rect locateQRCode(const Mat &source,int cannyValue,int blurValue,bool isHisEqual);
    void setPixelTolerance(float t){
        pixel_tolerance = t;
    }
};

const int QUALITY_BAD = 1;
const int QUALITY_MEDIUM = 2;
const int QUALITY_EXCELLENT = 3;
struct CandidateRegion{

public:
    std::vector<int> adjacentIndex;
    std::vector<Point> contour;
    cv::RotatedRect rect;
    int quality = QUALITY_BAD;
    int selfIndex;

    void addAdjacentRegion(int index){

        if (index == selfIndex){
            return;
        }

        if(std::find(adjacentIndex.begin(),adjacentIndex.end(),index) != adjacentIndex.end()){
            return;
        }

        adjacentIndex.push_back(index);
    }

    cv::RotatedRect mergeRegions(std::vector<CandidateRegion> region){
        if (adjacentIndex.empty()){
            return rect;
        }

        std::vector<cv::Point> finalContours(contour);
        std::for_each(adjacentIndex.begin(),adjacentIndex.end(),[&](int index){
            CandidateRegion r = region[index];
           finalContours.insert(finalContours.end(),r.contour.begin(),r.contour.end());
        });
        return minAreaRect(finalContours);
    }

};

#endif //CZXING_QRCODEFINDER_H
