//
// Created by uchia on 2019-10-02.
//

#include "QRCodeFinder.h"
#include "JNIUtils.h"

bool sortByArea(const std::vector<Point> &v1, const std::vector<Point> &v2) {
    cv::Rect v1Area = boundingRect(v1);
    cv::Rect v2Area = boundingRect(v2);
    return v1Area.area() > v2Area.area();
}

void drawMinRect(cv::Mat &imageContours, std::vector<cv::Point> &contour){
    RotatedRect rect=minAreaRect(contour);
    Point2f P[4];
    rect.points(P);
    for(int j=0;j<=3;j++)
    {
        line(imageContours,P[j],P[(j+1)%4],Scalar(255,0,0),1);
    }
}

cv::Mat QRCodeFinder::locateQRCode(
        const Mat &source,
        int cannyValue,
        int blurValue,
        bool isHisEqual) {

    Mat grayMat;
    cvtColor(source, grayMat, CV_BGR2GRAY);
    if (isHisEqual) {
        equalizeHist(grayMat, grayMat);
    }
    Mat thresholdMat;
    threshold(grayMat, thresholdMat, 0, 255, CV_THRESH_OTSU);
    Mat preResultMat = preProcessMat(grayMat, cannyValue, blurValue);

    std::vector<std::vector<Point>> contours;
    std::vector<Vec4i> hierarchy;
    std::vector<std::vector<Point>> potentialContours;
    findContours(preResultMat, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());
    for (int i = 0; i < contours.size(); ++i) {
        int index = i;
        int childContours = 0;
        while (hierarchy[index][2] != -1) {
            index = hierarchy[index][2];
            childContours++;
            if (childContours > 3) {
                potentialContours.push_back(contours[i]);
                break;
            }
        }
    }
    Mat result = Mat::zeros(thresholdMat.size(), CV_8UC3);;
    std::vector<float> angles;
    for (int i = 0; i < potentialContours.size(); i++) {
        RotatedRect rect = minAreaRect(potentialContours[i]);
        if (rect.size.area() < 200) {
            continue;
        }

//        if (rect.angle == 0) {
//            Point2f points[4];
//            rect.points(points);
//            Rect r(points[1].x,points[1].y,rect.size.width,rect.size.height);
//            checkPositionDetectionPattern(thresholdMat,r);
//        } else {
//            rotateArea(thresholdMat,rect);
//        }
        drawContours(result, potentialContours, i, Scalar(255));
        drawMinRect(result,potentialContours[i]);
    }

    return result;

//    sort(potentialContours.begin(),potentialContours.end(),sortByArea);
//    drawMinRect(result,potentialContours[0]);
//    return result;

//    sort(potentialContours.begin(),potentialContours.end(),sortByArea);
//    return rotateArea(thresholdMat,minAreaRect(potentialContours[0]));
}

cv::Mat QRCodeFinder::preProcessMat(
        const Mat &grayMat,
        int cannyValue,
        int blurValue) {

    Mat blurMat;
    GaussianBlur(grayMat, blurMat, Size(blurValue, blurValue), 0);
    Mat cannyMat;
    Canny(blurMat, cannyMat, 100, cannyValue, 3);
    Mat thresholdMat;
    threshold(cannyMat, thresholdMat, 0, 255, CV_THRESH_OTSU);
    return thresholdMat;
}

cv::Mat QRCodeFinder::rotateArea(cv::Mat &source,cv::RotatedRect area){

    std::vector<cv::Point2f> destArea;
    destArea.push_back(Point2f(0,0));
    destArea.push_back(Point2f(area.size.width - 1, 0));
    destArea.push_back(Point2f(area.size.width - 1, area.size.height -1));
    destArea.push_back(Point2f(0, area.size.height - 1));


    Point2f points[4];
    area.points(points);
    std::vector<cv::Point2f> sourceArea;
    sourceArea.push_back(points[1]);
    sourceArea.push_back(points[2]);
    sourceArea.push_back(points[3]);
    sourceArea.push_back(points[0]);


    Mat result = Mat::zeros(Size(area.boundingRect().width, area.boundingRect().height),CV_8UC3);
    Mat h = findHomography(sourceArea,destArea,RANSAC);
    warpPerspective(source, result, h, source.size());
    return result;

}

bool
QRCodeFinder::checkPositionDetectionPattern(const cv::Mat &source, cv::Rect area) {

//    drawThisMinAreaRect(source,rect);
    return true;
}