//
// Created by uchia on 2019-12-08.
//

#include "ZXingHooker.h"
#include "JNIUtils.h"
#include <opencv2/core/mat.hpp>
#include <src/MultiFormatReader.h>
#include <src/BinaryBitmap.h>
#include "BitMatrix.h"
#include "android_utils.h"

using namespace cv;
using namespace ZXing;

cv::Mat convertBitMatrixToMat(long matrixPtr) {
    auto ptr = reinterpret_cast<BitMatrix *>(matrixPtr);
    auto width = ptr->width();
    auto height = ptr->height();
    cv::Mat image(height, width, CV_8UC1);

    for (auto y = 0; y < height; y++) {
        for (auto x = 0; x < width; x++) {
            auto value = ptr->get(x, y);
            image.at<unsigned char>(y, x) = value ? 0 : 255;
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
    if (before){
        fileName.append("before-sampling-");
    } else {
        fileName.append("after-sampling-");
    }
    writeImage(image,path,fileName);
}

void ZXingHooker::hookHandler(int phrase, long p1, long p2) const {
    switch (phrase) {
        case HookPhrase::HOOK_THRESHOLD: {
            handleThreshold(p1, p2);
            break;
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