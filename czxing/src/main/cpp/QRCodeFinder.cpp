//
// Created by uchia on 2019-10-02.
//

#include <numeric>
#include <cmath>
#include <utility>

#include "QRCodeFinder.h"
#include "JNIUtils.h"
#include "android_utils.h"

static bool sortByDistance(const std::pair<Point2f, Point2f> &p1,
                           const std::pair<Point2f, Point2f> &p2) {
    double d1 = (p1.first.x - p1.second.x) * (p1.first.x - p1.second.x)
                + (p1.first.y - p1.second.y) * (p1.first.y - p1.second.y);
    double d2 = (p2.first.x - p2.second.x) * (p2.first.x - p2.second.x)
                + (p2.first.y - p2.second.y) * (p2.first.y - p2.second.y);
    return sqrt(d1) - sqrt(d2) < 0;
}

static double calculateVariance(std::vector<int> vector) {
    double sum = std::accumulate(std::begin(vector), std::end(vector), 0.0);
    double mean = sum / vector.size(); //均值

    double accum = 0.0;
    std::for_each(std::begin(vector), std::end(vector), [&](const double d) {
        accum += (d - mean) * (d - mean);
    });

    return sqrt(accum / (vector.size() - 1)); //方差
}

void drawMinRect(cv::Mat &imageContours, std::vector<cv::Point> &contour) {
    RotatedRect rect = minAreaRect(contour);
    Point2f P[4];
    rect.points(P);
    for (int j = 0; j <= 3; j++) {
        line(imageContours, P[j], P[(j + 1) % 4], Scalar(255, 0, 0), 1);
    }
}

void drawTimingPattern(cv::Mat &source, LineIterator &l, const cv::RotatedRect &area) {
    int skipInterval = (int) (max(area.size.width, area.size.height) / 14 + 0.5);
    for (int i = 0; i < (l.count - skipInterval); ++i, ++l) {
        if (i <= skipInterval) {
            continue;
        }
        circle(source, l.pos(), 2, Scalar(128), -1);
    }
}

void addOffsetForPoint(cv::Point2f &raw, const cv::RotatedRect &rect) {

    Point2f center = rect.center;
    float xOffset = rect.size.width / 14;
    float yOffset = rect.size.height / 14;

    if (raw.x <= center.x && raw.y <= center.y) {
        raw.x += xOffset;
        raw.y += yOffset;
    } else if (raw.x > center.x && raw.y <= center.y) {
        raw.x -= xOffset;
        raw.y += yOffset;
    } else if (raw.x > center.x && raw.y > center.y) {
        raw.x -= xOffset;
        raw.y -= yOffset;
    } else if (raw.x <= center.x && raw.y > center.y) {
        raw.x += xOffset;
        raw.y -= yOffset;
    }

}

bool candidateEqual(const CandidateRegion &p1, const CandidateRegion &p2) {
    RotatedRect r1 = p1.rect;
    RotatedRect r2 = p2.rect;
    if (r1.size.width != r2.size.width || r1.size.height != r2.size.height) {
        return false;
    }
    if (r1.center.x != r2.center.x || r1.center.y != r2.center.y) {
        return false;
    }
    return r1.angle == r2.angle;
}

cv::Rect QRCodeFinder::locateQRCode(
        const Mat &source,
        int cannyValue,
        int blurValue,
        bool isHisEqual) {

//    Mat grayMat;
//    cvtColor(source, grayMat, CV_BGR2GRAY);
//    if (isHisEqual) {
//        equalizeHist(grayMat, grayMat);
//    }
//    Mat thresholdMat;
//    threshold(source, thresholdMat, 0, 255, CV_THRESH_OTSU);
//    writeImage(source,"source-");
    Mat preResultMat = preProcessMat(source, cannyValue, blurValue);
//    writeImage(preResultMat,"pre-process");
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
            if (childContours >= 3) {
                potentialContours.push_back(contours[i]);
                break;
            }
        }
    }
    // 过滤掉内接轮廓
    filterInscribedRect(potentialContours);
    Mat result = Mat::zeros(source.size(), CV_8UC3);
    std::vector<CandidateRegion> candidates;

    // 计算矩形是为position pattern
    for (int i = 0; i < potentialContours.size(); i++) {
        RotatedRect rect = minAreaRect(potentialContours[i]);
        if (rect.size.area() < 200) {
            continue;
        }
        CandidateRegion candidateRegion;
        candidateRegion.rect = rect;
        candidateRegion.contour = potentialContours[i];
        candidateRegion.selfIndex = i;
        if (checkPositionDetectionPattern(source, rect)) {
            candidateRegion.quality = QUALITY_MEDIUM;
//            drawMinRect(result, potentialContours[i]);
        }
        drawContours(result,potentialContours,i,Scalar(255,0,0));
        candidates.push_back(candidateRegion);
    }

    writeImage(result,"locateQRCode-");
    // 计算矩形之间的 timing pattern
    for (int i = 0; i < candidates.size(); i++) {
        for (int j = i + 1; j < candidates.size(); j++) {
            RotatedRect r = candidates[i].rect;
            RotatedRect r2 = candidates[j].rect;
            std::vector<std::pair<cv::Point2f, cv::Point2f>> alignResult
                    = checkTimingPattern(source, r, r2);

            if (alignResult.empty()) {
                continue;
            }

            if (candidates[i].quality >= QUALITY_MEDIUM) {
                candidates[i].quality = QUALITY_EXCELLENT;
            } else {
                candidates[i].quality = QUALITY_MEDIUM;
            }

            if (candidates[j].quality >= QUALITY_MEDIUM) {
                candidates[j].quality = QUALITY_EXCELLENT;
            } else {
                candidates[j].quality = QUALITY_MEDIUM;
            }

            candidates[i].addAdjacentRegion(j);
            candidates[j].addAdjacentRegion(i);

//            std::for_each(alignResult.begin(),
//                          alignResult.end(),
//                          [&](const std::pair<cv::Point2f, cv::Point2f> l1) {
//                              line(result, l1.first, l1.second, Scalar(255, 0, 0), 2);
//                          });

        }
    }



    Rect region = findRegion(candidates);
//    if (region.size.area() > 0){
//        Point2f P[4];
//        region.points(P);
//        for (int j = 0; j <= 3; j++) {
//            line(result, P[j], P[(j + 1) % 4], Scalar(0, 255, 0), 3);
//        }
//    }

    return region;
}

cv::Rect QRCodeFinder::findRegion(std::vector<CandidateRegion> &candidates){

    std::vector<CandidateRegion> v(candidates);
    std::sort(v.begin(),
              v.end(),
              [](const CandidateRegion &p, const CandidateRegion &q) {
                  if (p.quality > q.quality){
                      return true;
                  }
                  return p.adjacentIndex.size() > q.adjacentIndex.size();
              });

    // 既找到二维码的 Position Detection Pattern 又找到 Timing Pattern 基本可以确定二维码是可以识别的
    RotatedRect result(Point2f(0,0),Size2f(0,0),0);
    for (int i = 0; i < v.size(); i++){
        if (v[i].quality != QUALITY_EXCELLENT){
            continue;
        }
        if(v[i].adjacentIndex.size() <= 1){
            continue;
        }
        RotatedRect rect = v[i].mergeRegions(candidates);
        if(result.size.area() < rect.size.area()){
            result = rect;
        }
    }
    if (result.size.area() > 0){
        return result.boundingRect();
    }
//
//    // 没有找到 Position Detection Pattern 寻找拥有 Timing Pattern 线最多的顶点。
//    // 在比较远的情况下 Timing Pattern 基本存在并且可能存在多条
//    std::sort(v.begin(),v.end(),
//              [](const CandidateRegion &p, const CandidateRegion &q){
//                  if(p.adjacentIndex.size() > q.adjacentIndex.size()){
//                      return true;
//                  }
//                  return p.quality > q.quality;
//    });
//
//    for (int i = 0; i < v.size(); i++){
//        if (v[i].adjacentIndex.size() <= 1){
//            continue;
//        }
//        RotatedRect rect = v[i].mergeRegions(candidates);
//        if (result.size.area() < rect.size.area()){
//            result = rect;
//        }
//    }
//
//    if(result.size.empty()){
        return Rect(0,0,0,0);
//    }
//
//    return result.boundingRect();
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

//cv::Mat QRCodeFinder::rotateArea(const cv::Mat &source, cv::RotatedRect area) {
//
//    std::vector<cv::Point2f> destArea;
//    destArea.push_back(Point2f(0, 0));
//    destArea.push_back(Point2f(area.size.width - 1, 0));
//    destArea.push_back(Point2f(area.size.width - 1, area.size.height - 1));
//    destArea.push_back(Point2f(0, area.size.height - 1));
//
//
//    Point2f points[4];
//    area.points(points);
//    std::vector<cv::Point2f> sourceArea;
//    sourceArea.push_back(points[1]);
//    sourceArea.push_back(points[2]);
//    sourceArea.push_back(points[3]);
//    sourceArea.push_back(points[0]);
//
//
//    Mat result = Mat::zeros(Size(area.boundingRect().width, area.boundingRect().height), CV_8UC1);
//    Mat h = findHomography(sourceArea, destArea, RANSAC);
//    warpPerspective(source, result, h, source.size());
////    rectangle(result,Rect(20,20,rect.size.width,rect.size.height),Scalar(128));
//    return result;
//}

bool QRCodeFinder::checkPositionDetectionPattern(const cv::Mat &source, cv::RotatedRect area) {
    bool typeMatch = source.type() == CV_8UC1;
    if (!typeMatch) {
        return false;
    }

    Point2f points[4];
    area.points(points);
    Point2f startPoint, endPoint;
    startPoint.x = (points[0].x + points[3].x) / 2;
    startPoint.y = (points[0].y + points[3].y) / 2;
    endPoint.x = (points[1].x + points[2].x) / 2;
    endPoint.y = (points[1].y + points[2].y) / 2;


    LineIterator vLine(source, startPoint, endPoint);
    if (check11311Pattern(source, vLine)) {
        LineIterator l(source, startPoint, endPoint);
        for (int i = 0; i < (l.count); ++i, ++l) {
            circle(source, l.pos(), 2, Scalar(128), -1);
        }
        return true;
    }

    return false;
}

/**
 * 过滤掉内接轮廓
 * @param source
 */
void QRCodeFinder::filterInscribedRect(std::vector<std::vector<cv::Point>> &source) {
    std::vector<cv::RotatedRect> areas;
    for (int i = 0; i < source.size(); i++) {
        cv::RotatedRect r = minAreaRect(source[i]);
        areas.push_back(r);
    }

    std::vector<std::vector<cv::Point>> vector(source);
    for (int i = 0; i < vector.size(); ++i) {
        RotatedRect currentArea = areas[i];
        if (source[i].empty()) {
            continue;
        }
        for (int j = i + 1; j < vector.size(); ++j) {
            RotatedRect nextArea = areas[j];

            if (abs(nextArea.center.x - currentArea.center.x) > DEVIATION_PIXEL ||
                abs(nextArea.center.y - currentArea.center.y) > DEVIATION_PIXEL) {
                continue;
            }
            int ca = currentArea.size.height * currentArea.size.width;
            int na = nextArea.size.width * nextArea.size.height;
            if (abs(ca - na) > 2 * DEVIATION_PIXEL) {
                continue;
            }
            std::vector<cv::Point> emptyVector;
            if (ca >= na) {
                source[j].swap(emptyVector);
                break;
            } else {
                source[i].swap(emptyVector);
                break;
            }
        }
    }

    std::vector<std::vector<Point>>::iterator it = source.begin();
    while (it != source.end()) {
        std::vector<Point> v = *it;
        if (v.empty()) {
            it = source.erase(it);
        } else {
            ++it;
        }
    }
}

/**
 * 通过 1:1:3:1:1确定Position Pattern
 * @param source
 * @param l
 * @return
 */
bool QRCodeFinder::check11311Pattern(const cv::Mat &source, LineIterator &l) {

    int totalCount = l.count;

    // 跳过白边
    int start = 0;
    while ((int) source.at<uchar>(l.pos()) == canTolerate(255,255)) {
        ++l;
        ++start;
    }

    // 统计 1:1:3:1:1 各个区块的像素个数
    float phase[] = {0, 0, 0, 0, 0};
    int currentPhase = 0;
    for (int i = start; i < totalCount; ++i, ++l) {
        int value = (int) source.at<uchar>(l.pos());
        if (currentPhase % 2 == 0 && canTolerate(0,value)) {
            phase[currentPhase] += 1;
        } else if (currentPhase % 2 == 1 && canTolerate(255,value)) {
            phase[currentPhase] += 1;
        } else {
            ++currentPhase;
            if (currentPhase >= 5) {
                break;
            }
            phase[currentPhase] += 1;
        }
    }

    float positionPatternPixelCount = std::accumulate(phase, phase + 5, 0);

    // 如果 1:1:3:1:1 占比过小则认为该区域不是 position pattern
    if (positionPatternPixelCount / totalCount < 0.95f) {
        return false;
    }

    float seventh = 1.0f / 7;
    float threeSeventh = 3.0f / 7;

    if (abs(phase[0] / positionPatternPixelCount - seventh) >= DEVIATION_RATIO) {
        return false;
    }

    if (abs(phase[1] / positionPatternPixelCount - seventh) >= DEVIATION_RATIO) {
        return false;
    }

    if (abs(phase[2] / positionPatternPixelCount - threeSeventh) >= DEVIATION_RATIO) {
        return false;
    }

    if (abs(phase[3] / positionPatternPixelCount - seventh) >= DEVIATION_RATIO) {
        return false;
    }

    if (abs(phase[4] / positionPatternPixelCount - seventh) >= DEVIATION_RATIO) {
        return false;
    }

    return true;
}

/**
 * 通过黑白像素比例是否接近确认是否为 Timing Pattern
 * @param source
 * @param l
 * @return
 */
bool checkTimingPatternInternal(const cv::Mat &source, cv::LineIterator &l) {
    int totalCount = l.count;
    std::vector<int> result;
    result.push_back(0);
    int currentPhase = 0;
    for (int i = 0; i < l.count; ++i, ++l) {
        int value = (int) source.at<uchar>(l.pos());
        if (currentPhase % 2 == 0 && value == 0) {
            result[currentPhase] += 1;
        } else if (currentPhase % 2 == 1 && value == 255) {
            result[currentPhase] += 1;
        } else {
            ++currentPhase;
            result.push_back(1);
        }
    }

    int size = result.size();
    if (size <= 4) {
        return false;
    }

    int i = 0;
    std::vector<int>::iterator iterator = result.begin();
    while (iterator != result.end()) {
        if (i == 0 || i == 1 || i == size - 2 || i == size - 1) {
            iterator = result.erase(iterator);
        }
        ++iterator;
        ++i;
    }

    if (result.size() < 5) {
        return false;
    }

    double variance = calculateVariance(result);
    return variance <= 5;
}

std::vector<std::pair<cv::Point2f, cv::Point2f>>
QRCodeFinder::checkTimingPattern(const cv::Mat &source, const RotatedRect &r1, const RotatedRect &r2) {

    Point2f p1[4];
    r1.points(p1);

    Point2f p2[4];
    r2.points(p2);

    std::vector<std::pair<Point2f, Point2f>> potentialPairs;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            potentialPairs.push_back(std::pair<Point2f, Point2f>(p1[i], p2[j]));
        }
    }

    std::sort(potentialPairs.begin(), potentialPairs.end(), sortByDistance);

    addOffsetForPoint(potentialPairs[0].first, r1);
    addOffsetForPoint(potentialPairs[0].second, r2);

    addOffsetForPoint(potentialPairs[1].first, r1);
    addOffsetForPoint(potentialPairs[1].second, r2);

    std::vector<std::pair<cv::Point2f, cv::Point2f>> pairs;
    LineIterator l(source, potentialPairs[0].first, potentialPairs[0].second);
    if (checkTimingPatternInternal(source, l)) {
//        LineIterator ll(source, potentialPairs[0].first, potentialPairs[0].second);
//        drawTimingPattern(source, ll, r1);
        pairs.push_back(potentialPairs[0]);
    }

    LineIterator l2(source, potentialPairs[1].first, potentialPairs[1].second);
    if (checkTimingPatternInternal(source, l2)) {
//        LineIterator ll2(source, potentialPairs[1].first, potentialPairs[1].second);
//        drawTimingPattern(source, ll2, r1);
        pairs.push_back(potentialPairs[1]);
    }
    return pairs;
}

bool QRCodeFinder::canTolerate(int basePixel,int currentPixel){
    int minPixel = MAX(0,basePixel - 255 * pixel_tolerance);
    int maxPixel = MIN(255, basePixel + 255 * pixel_tolerance);

    return currentPixel >= minPixel && currentPixel <= maxPixel;
}