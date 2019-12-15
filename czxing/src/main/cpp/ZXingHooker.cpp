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

using namespace cv;
using namespace ZXing;

static int currentVersionNum = -1;

cv::Mat convertBitMatrixToMat(long matrixPtr) {
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

void ZXingHooker::handleThreshold(long matrixPtr, long p2) const {
    auto image = convertBitMatrixToMat(matrixPtr);
    std::string path = "/storage/emulated/0/scan/threshold/";
    writeImage(image, path, "threshold-hook-");
}

void ZXingHooker::handleGirdSampling(long matrixPtr, bool before) const {
    auto image = convertBitMatrixToMat(matrixPtr);
    std::string path = "/storage/emulated/0/scan/sampling/";
    std::string fileName;
    if (before) {
        fileName.append("before-sampling-");
    } else {
        fileName.append("after-sampling-");
    }
    writeImage(image, path, fileName);
}

void drawProtentialAlimentArea(Mat &mat,
                                float overallEstModuleSize,
                                int estAlignmentX,
                                int estAlignmentY,
                                int width,
                                int height,
                                float allowanceFactor){

        // Look for an alignment pattern (3 modules in size) around where it
        // should be
        int allowance = (int) (allowanceFactor * overallEstModuleSize);
        int alignmentAreaLeftX = std::max(0, estAlignmentX - allowance);
        int alignmentAreaRightX = std::min(width - 1, estAlignmentX + allowance);
        if (alignmentAreaRightX - alignmentAreaLeftX < overallEstModuleSize * 3) {
            return;
        }

        int alignmentAreaTopY = std::max(0, estAlignmentY - allowance);
        int alignmentAreaBottomY = std::min(height - 1, estAlignmentY + allowance);
        if (alignmentAreaBottomY - alignmentAreaTopY < overallEstModuleSize * 3) {
            return;
        }
}

void ZXingHooker::handleFindPositionPattern(long matrixPtr, long finderPatternInfoPtr,
                                            long alignPatternPtr) const {

    if (finderPatternInfoPtr == 0) {
        return;
    }

    std::string path = "/storage/emulated/0/scan/findpattern/";

    auto mat = convertBitMatrixToMat(matrixPtr);
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

    // 画出计算得到的 alignmentX alignmentY
    cv::circle(mat, cv::Point(estAlignmentX, estAlignmentY), 4, cv::Scalar(255, 255,0), -1);

    if (alignPatternPtr != 0) {
        auto alignPattern = reinterpret_cast<QRCode::AlignmentPattern *>(alignPatternPtr);
        if (alignPattern->isValid()) {
            cv::circle(mat, cv::Point(alignPattern->x(), alignPattern->y()), 4,
                       cv::Scalar(0, 255, 255), -1);
        }
    }

    writeImage(mat, path, "find-pattern-");
}

void ZXingHooker::handleCalculateVersion(long versionPtr) const {
    if (versionPtr == 0) {
        return;
    }

    auto version = reinterpret_cast<QRCode::Version *>(versionPtr);
    auto dimension = version->dimensionForVersion();
    auto versionNum = version->versionNumber();

    currentVersionNum = versionNum;

    LOGE("qrcode version num: %d dimension: %d x %d", versionNum, dimension, dimension);
}

void ZXingHooker::hookHandler(int phrase, long p1, long p2, long p3) const {
    switch (phrase) {
        case HookPhrase::HOOK_THRESHOLD: {
            handleThreshold(p1, p2);
            break;
        }
        case HookPhrase::HOOK_CALCULATE_VERSION : {
            handleCalculateVersion(p1);
            break;
        }
        case HookPhrase::HOOK_PERSPECTIVE_TRANSFORM: {
            handleFindPositionPattern(p1, p2, p3);
        }
        case HookPhrase::HOOK_BEFORE_GIRD_SAMPLING : {
            handleGirdSampling(p1, true);
            break;
        }
        case HOOK_AFTER_GIRD_SAMPLING: {
            handleGirdSampling(p1, false);
            break;
        }
        default:
            break;
    }
}