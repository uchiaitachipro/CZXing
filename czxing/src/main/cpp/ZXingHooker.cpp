//
// Created by uchia on 2019-12-08.
//

#include "ZXingHooker.h"
#include "JNIUtils.h"
#include <opencv2/core/mat.hpp>
#include <src/MultiFormatReader.h>
#include <src/BinaryBitmap.h>
#include <src/qrcode/QRFinderPatternInfo.h>
#include <src/qrcode/QRAlignmentPattern.h>
#include <src/ResultPoint.h>
#include <src/qrcode/QRDetector.h>
#include "BitMatrix.h"
#include "android_utils.h"
#include "CopyOfFindAlignmentPattern.h"
#include <src/qrcode/QRStrictAlignmentPatternFinder.h>

using namespace cv;
using namespace ZXing;
using namespace std;

static int currentVersionNum = -1;
static float currentModuleSize = 0.0f;
std::vector<cv::Point> pixels;


void CollectData(int x,int y){
    pixels.push_back(cv::Point(x, y));
}

cv::Mat ConvertBitMatrixToMat(long matrixPtr) {
    auto ptr = reinterpret_cast<BitMatrix *>(matrixPtr);
    auto width = ptr->width();
    auto height = ptr->height();
    cv::Mat image(height, width, CV_8UC3);

    for (auto y = 0; y < height; y++) {
        for (auto x = 0; x < width; x++) {
            auto value = ptr->get(x, y);
            image.at<cv::Vec3b>(y, x) = value ? cv::Vec3b(0, 0, 0) : cv::Vec3b(255, 255, 255);
        }
    }
    return image;
}

void ZXingHooker::HandleThreshold(long matrixPtr, long p2) const {
    auto image = ConvertBitMatrixToMat(matrixPtr);
    std::string path = "/storage/emulated/0/scan/threshold/";
    writeImage(image, path, "threshold-hook-");
}

void ZXingHooker::HandleSaveModuleSize(long valuePtr) const{
    if (valuePtr == 0){
        return;
    }
    auto value = reinterpret_cast<float *>(valuePtr);
    currentModuleSize  = *value;
    LOGE("current module size:  %f",currentModuleSize);
}

void ZXingHooker::HandleGirdSampling(long matrixPtr, bool before) const {
    auto image = ConvertBitMatrixToMat(matrixPtr);
    std::string path = "/storage/emulated/0/scan/sampling/";
    std::string fileName;
    if (before) {
        fileName.append("before-sampling-");
    } else {
        fileName.append("after-sampling-");
    }
    writeImage(image, path, fileName);
}


void ZXingHooker::HandleFindPositionPattern(long matrixPtr, long finderPatternInfoPtr,
                                            long alignPatternPtr) const {

    if (finderPatternInfoPtr == 0) {
        return;
    }

    std::string path = "/storage/emulated/0/scan/findpattern/";

    auto mat = ConvertBitMatrixToMat(matrixPtr);
    auto findPattern = reinterpret_cast<QRCode::FinderPatternInfo *>(finderPatternInfoPtr);
    auto topLeft = findPattern->topLeft;
    auto topRight = findPattern->topRight;
    auto bottomLeft = findPattern->bottomLeft;

    cv::circle(mat, cv::Point(topLeft.x(), topLeft.y()), 4, cv::Scalar(0, 0, 255), -1);
    cv::circle(mat, cv::Point(topRight.x(), topRight.y()), 4, cv::Scalar(0, 0, 255), -1);
    cv::circle(mat, cv::Point(bottomLeft.x(), bottomLeft.y()), 4, cv::Scalar(0, 0, 255), -1);

    // Guess where a "bottom right" finder pattern would have been
    float bottomRightX = topRight.x() - topLeft.x() + bottomLeft.x();
    float bottomRightY = topRight.y() - topLeft.y() + bottomLeft.y();

    // 画出计算得到的bottom right 位置
    cv::circle(mat, cv::Point(bottomRightX, bottomRightY), 4, cv::Scalar(0, 0, 255), -1);

    auto currentVersion = QRCode::Version::VersionForNumber(currentVersionNum);

    if (currentVersion == NULL || currentVersion->alignmentPatternCenters().empty()) {
        return;
    }

    int modulesBetweenFPCenters = currentVersion->dimensionForVersion() - 7;
    // Estimate that alignment pattern is closer by 3 modules
    // from "bottom right" to known top left location
    float correctionToTopLeft = 1.0f - 3.0f / (float) modulesBetweenFPCenters;
    int estAlignmentX = static_cast<int>(topLeft.x() + correctionToTopLeft *
                                                            (bottomRightX -
                                                             topLeft.x()));
    int estAlignmentY = static_cast<int>(topLeft.y() + correctionToTopLeft *
                                                            (bottomRightY -
                                                             topLeft.y()));

////    // 画出计算得到的 alignmentX alignmentY
////    cv::circle(mat, cv::Point(estAlignmentX, estAlignmentY), 4, cv::Scalar(255, 255,0), -1);
//
//    if (currentModuleSize > 0){
//        for (int i = 4; i <= 16; i <<= 1){
//            findAlignmentInRegion(mat,
//                                  *reinterpret_cast<BitMatrix *>(matrixPtr),
//                                  currentModuleSize,
//                                  estAlignmentX,
//                                  estAlignmentY,
//                                  static_cast<float>(i));
//        }
//
//        pixels = {};
//        for (int i = 4;i <= 16; i<<=1){
//            auto image = reinterpret_cast<BitMatrix *>(matrixPtr);
//            int allowance = (int) (i * currentModuleSize);
//            int alignmentAreaLeftX = std::max(0, estAlignmentX - allowance);
//            int alignmentAreaRightX = std::min(image->width() - 1, estAlignmentX + allowance);
//            if (alignmentAreaRightX - alignmentAreaLeftX < currentModuleSize * 3) {
//                continue;
//            }
//
//            int alignmentAreaTopY = std::max(0, estAlignmentY - allowance);
//            int alignmentAreaBottomY = std::min(image->height() - 1, estAlignmentY + allowance);
//            if (alignmentAreaBottomY - alignmentAreaTopY < currentModuleSize * 3) {
//                continue;
//            }
//            auto result = StrictAlignmentPatternFinder::Find(*image,
//                                               alignmentAreaLeftX, alignmentAreaTopY,
//                                               alignmentAreaRightX - alignmentAreaLeftX,
//                                               alignmentAreaBottomY - alignmentAreaTopY,
//                                               currentModuleSize,CollectDiagonalData);
//            if (result.isValid()){
//                cv::circle(mat, cv::Point(result.x(), result.y()), 2, cv::Scalar(255, 0,255), -1);
//                break;
//            }
//        }
//        std::for_each(pixels.begin(),pixels.end(),[&](cv::Point p){
//            cv::circle(mat, p, 1, cv::Scalar(128, 128,128), -1);
//        });
////    }

    if (alignPatternPtr != 0) {
        auto alignPattern = reinterpret_cast<QRCode::AlignmentPattern *>(alignPatternPtr);
        if (alignPattern->isValid()) {
            cv::circle(mat, cv::Point(alignPattern->x(), alignPattern->y()), 4,
                       cv::Scalar(0, 255, 255), -1);
        }
    }

    writeImage(mat, path, "find-pattern-");
}

void ZXingHooker::HandleCalculateVersion(long versionPtr) const {
    if (versionPtr == 0) {
        return;
    }

    auto version = reinterpret_cast<QRCode::Version *>(versionPtr);
    auto dimension = version->dimensionForVersion();
    auto versionNum = version->versionNumber();

    currentVersionNum = versionNum;

    LOGE("qrcode version num: %d dimension: %d x %d", versionNum, dimension, dimension);
}

void ZXingHooker::HookHandler(int phrase, long p1, long p2, long p3) const {
    switch (phrase) {
        case HookPhrase::HOOK_THRESHOLD: {
            HandleThreshold(p1, p2);
            break;
        }
        case HookPhrase::HOOK_CALCULATE_VERSION : {
            HandleCalculateVersion(p1);
            break;
        }
        case HookPhrase::HOOK_FIND_POSITION_PATTERN: {
            HandleFindPositionPattern(p1, p2, p3);
        }
        case HookPhrase::HOOK_BEFORE_GIRD_SAMPLING : {
            HandleGirdSampling(p1, true);
            break;
        }
        case HookPhrase::HOOK_AFTER_GIRD_SAMPLING: {
            HandleGirdSampling(p1, false);
            break;
        }
        case HookPhrase::HOOK_MODULE_SIZE:{
            HandleSaveModuleSize(p1);
            break;
        }
        case HookPhrase::HOOK_FINISH_CONFIRM_POSITION:
        case HookPhrase::HOOK_CONFIRMING_POSITION:
        case HookPhrase::HOOK_START_CONFIRM_POSITION:{
//            HandleConfirmPositionPattern(phrase,p1,p2,p3);
            break;
        }
        default:
            break;
    }
}

void ZXingHooker::HandleConfirmPositionPattern(int phrase, long matrixPtr, long x, long y) const {
    if (phrase == ZXingHooker::HOOK_START_CONFIRM_POSITION){
        pixels.clear();
        return;
    }
    if (phrase == ZXingHooker::HOOK_CONFIRMING_POSITION){
        CollectData(x,y);
        return;
    }
    if (phrase == ZXingHooker::HOOK_FINISH_CONFIRM_POSITION){
        auto mat = ConvertBitMatrixToMat(matrixPtr);
        if (pixels.empty()){
            return;
        }
        std::string path = "/storage/emulated/0/scan/finding/";
        std::for_each(pixels.begin(),pixels.end(),[&](auto point){
            cv::circle(mat,point,1,Scalar(0,0,255));
            writeImage(mat,path,"finding-");
        });
    }
}