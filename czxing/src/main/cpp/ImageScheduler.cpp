//
// Created by Devilsen on 2019-08-09.
//

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <src/BinaryBitmap.h>
#include "ImageScheduler.h"
#include "JNIUtils.h"
#include "opencv2/highgui.hpp"
#include "android_utils.h"
#include "ThresholdTool.h"
#include <math.h>
#include <src/qrcode/QRReader.h>
#include <src/DecodeHints.h>
#include "ZXingHooker.h"

#define DEFAULT_MIN_LIGHT 70;


ImageScheduler::ImageScheduler(JNIEnv *env, MultiFormatReader *_reader,
                               JavaCallHelper *javaCallHelper) {
    this->env = env;
    this->reader = _reader;
    this->javaCallHelper = javaCallHelper;
    qrCodeRecognizer = new QRCodeRecognizer();
    stopProcessing.store(false);
    isProcessing.store(false);
    abortTask.store(false);
}

ImageScheduler::~ImageScheduler() {
    DELETE(env);
    DELETE(reader);
    DELETE(javaCallHelper);
    DELETE(qrCodeRecognizer);

    delete &isProcessing;
    delete &stopProcessing;
    delete &cameraLight;
    delete &prepareThread;
}


void *prepareMethod(void *arg) {
}

void ImageScheduler::prepare() {
}

void ImageScheduler::start() {
}

void ImageScheduler::stop() {
}

void
ImageScheduler::process(jbyte *bytes, int left, int top,
                        int cropWidth, int cropHeight, int rowWidth,
                        int rowHeight, int strategyIndex) {
    if (isProcessing.load()) {
        return;
    }

    FrameData frameData;
    frameData.left = left;
    frameData.top = top;
    if (left + cropWidth > rowWidth) {
        frameData.cropWidth = rowWidth - left;
    } else {
        frameData.cropWidth = cropWidth;
    }

    if (top + cropHeight > rowHeight) {
        frameData.cropHeight = rowHeight - top;
    } else {
        frameData.cropHeight = cropHeight;
    }

    frameData.rowWidth = rowWidth;
    frameData.rowHeight = rowHeight;
    frameData.bytes = bytes;
    currentStrategyIndex = strategyIndex;
//    frameQueue.enQueue(frameData);
    preTreatMat(frameData);
    LOGE("frame data size : %d", frameQueue.size());

}

/**
 * 预处理二进制数据
 */
void ImageScheduler::preTreatMat(const FrameData &frameData) {
    try {
        LOGE("start preTreatMat...");

        Mat src(frameData.rowHeight + frameData.rowHeight / 2,
                frameData.rowWidth, CV_8UC1,
                frameData.bytes);

        Mat gray;
        cvtColor(src, gray, COLOR_YUV2GRAY_NV21);

        if (frameData.left != 0) {
            gray = gray(
                    Rect(frameData.left, frameData.top, frameData.cropWidth, frameData.cropHeight));
        }

        // 分析亮度，如果亮度过低，不进行处理
        analysisBrightness(gray);
        if (cameraLight < 40) {
            return;
        }
        applyStrategy(gray);
//        writeImage(gray,"raw");
    } catch (const std::exception &e) {
        LOGE("preTreatMat error...");
    }
}

void ImageScheduler::applyStrategy(Mat &mat) {

    int startIndex = isApplyAllStrategies ? 0 : currentStrategyIndex;
    startIndex = startIndex <= 0 ? 0 : startIndex % _strategies.size();
    bool result = false;
    for (int i = startIndex; i < _strategies.size(); ++i) {

        switch (_strategies[i]) {
            case DecodeStrategy::STRATEGY_RAW_PICTURE: {
                Result grayResult = decodeGrayPixels(mat);
                result = grayResult.isValid();
                if (grayResult.isValid()) {
                    //        writeImage(gray,"gray-");
                    //        qrCodeFinder.locateQRCode(mat, 200, 5, false);
                    javaCallHelper->onResult(grayResult, cameraLight);
                }
            }

                break;
            case DecodeStrategy::STRATEGY_THRESHOLD: {
                Result thresholdResult = decodeThresholdPixels(mat);
                result = thresholdResult.isValid();
                if (result) {
                    javaCallHelper->onResult(thresholdResult, cameraLight);
                    //        Rect rect =  qrCodeFinder.locateQRCode(mat, 200, 5, false);
                    //        writeImage(mat, std::string("threshold-"));
                }
                //            writeImage(mat, std::string("threshold-"));
                break;
            }

            case DecodeStrategy::STRATEGY_ADAPTIVE_THRESHOLD_CLOSELY: {
                Result closeAdaptiveResult = decodeAdaptivePixels(mat, ADAPTIVE_THRESH_MEAN_C, 55,
                                                                  3);
                result = closeAdaptiveResult.isValid();
                if (result) {
                    javaCallHelper->onResult(closeAdaptiveResult, cameraLight);
                    //        Rect rect =  qrCodeFinder.locateQRCode(mat, 200, 5, false);
                    //        writeImage(mat, std::string("adaptive-threshold-ROI-"));
                }
                //    writeImage(lightMat, std::string("adaptive-close-threshold-"));
                break;
            }
            case DecodeStrategy::STRATEGY_HUANG_FUZZY :{
                Mat huang;
                rotate(mat, huang, ROTATE_90_COUNTERCLOCKWISE);
                int thresholdValue = GetHuangFuzzyThreshold(huang);
                Result huangFuzzyResult = decodePixels(huang,thresholdValue);
                break;
            }
            case DecodeStrategy::STRATEGY_ADAPTIVE_THRESHOLD_REMOTELY: {
                Result remoteAdaptiveResult = decodeAdaptivePixels(mat, ADAPTIVE_THRESH_GAUSSIAN_C,
                                                                   25, 5);
                result = remoteAdaptiveResult.isValid();
                if (result) {
                    javaCallHelper->onResult(remoteAdaptiveResult, cameraLight);
                    //        Rect rect =  qrCodeFinder.locateQRCode(mat, 200, 5, false);
                    //        writeImage(mat, std::string("adaptive-remote-threshold-ROI-"));
                }
                //    writeImage(lightMat, std::string("adaptive-threshold-"));
            }
                break;
            case DecodeStrategy::STRATEGY_COLOR_EXTRACT:
                break;
            default:
                break;
        }
        if (result || !isApplyAllStrategies) {
            break;
        }
    }

    if (!result) {
        recognizerQrCode(mat);
    }
}


Result ImageScheduler::decodeGrayPixels(Mat &gray) {
    LOGE("start GrayPixels...");

    Mat mat;
    rotate(gray, mat, ROTATE_90_CLOCKWISE);
//    int threshold = GetHuangFuzzyThreshold(gray);
    Result result = decodePixels(gray, -1);
    return result;
}

Result ImageScheduler::decodeThresholdPixels(Mat &gray) {
    LOGE("start ThresholdPixels...");

    Mat mat;
    rotate(gray, mat, ROTATE_90_COUNTERCLOCKWISE);
//    rotate(gray, mat, ROTATE_180);

    // 提升亮度
    if (cameraLight < 80) {
        mat.convertTo(mat, -1, 1.0, 30);
    }

    threshold(mat, mat, 50, 255, CV_THRESH_OTSU);
    Result result = decodePixels(mat);
    return result;
}

Result ImageScheduler::decodeAdaptivePixels(Mat &gray, int adaptiveMethod, int blockSize,
                                            int delta) {
    LOGE("start AdaptivePixels...");

    Mat mat;
    rotate(gray, mat, ROTATE_90_COUNTERCLOCKWISE);

    // 降低图片亮度
    Mat lightMat;
    mat.convertTo(lightMat, -1, 1.0, -60);

    adaptiveThreshold(lightMat, lightMat, 255, adaptiveMethod,
                      THRESH_BINARY, blockSize, delta);
    Result result = decodePixels(lightMat);
    return result;
}

void ImageScheduler::recognizerQrCode(const Mat &mat) {
    LOGE("start recognizerQrCode...");
    cv::Rect rect;
//    rotate(mat, mat, ROTATE_90_COUNTERCLOCKWISE);
//    rect = qrCodeFinder.locateQRCode(mat, 200, 5, false);
//
//    if (rect.empty()){
    qrCodeRecognizer->processData(mat, &rect);
//    }

    if (rect.empty()) {
        return;
    }

    ResultPoint point1(rect.tl().x, rect.tl().y);
    ResultPoint point2(rect.br().x, rect.tl().y);
    ResultPoint point3(rect.tl().x, rect.br().y);

    std::vector<ResultPoint> points;
    points.push_back(point1);
    points.push_back(point2);
    points.push_back(point3);

    Result result(DecodeStatus::NotFound);
    result.setResultPoints(std::move(points));

    javaCallHelper->onResult(result, cameraLight);

    LOGE("end recognizerQrCode...");

}

Result ImageScheduler::decodePixels(Mat &mat, int threshold) {

    switch (detectType) {
        case DetectorType::ZBAR : {
            return decodeZBar(mat, threshold);
        }
        case DetectorType::ALL: {
            Result r = decodeZXing(mat, threshold);
            if (r.isValid()) {
                return r;
            }
            return decodeZBar(mat, threshold);
        }
        case DetectorType::PURE_ZXING:{
            return hookZXing(mat,threshold);
        }
        case DetectorType::ZXING :
        default: {
            return decodeZXing(mat, threshold);
        }
    }
}

bool ImageScheduler::analysisBrightness(const Mat &gray) {
    LOGE("start analysisBrightness...");

    // 平均亮度
    Scalar scalar = mean(gray);
    cameraLight = scalar.val[0];
    LOGE("平均亮度 %lf", cameraLight);
    // 判断在时间范围 AMBIENT_BRIGHTNESS_WAIT_SCAN_TIME * lightSize 内是不是亮度过暗
    bool isDark = cameraLight < DEFAULT_MIN_LIGHT;
    javaCallHelper->onBrightness(isDark);

    return isDark;
}

Result ImageScheduler::readBitmap(const cv::Mat &mat, int left, int top, int width, int height) {
    cv::Mat cropMat = mat(Rect(left, top, width, height));
    Mat gray;
    cvtColor(cropMat, gray, COLOR_RGB2GRAY);
//    writeImage(gray, "convert-gray-");
    bool result = false;

//    Mat closelyMat;
//    adaptiveThreshold(gray, closelyMat, 255, ADAPTIVE_THRESH_MEAN_C,
//                      THRESH_BINARY, 55, 3);
////    writeImage(closelyMat,"closelyMat-");
//    Result closelyAdaptiveResult = decodePixels(closelyMat, -1);
//    result = closelyAdaptiveResult.isValid();
//    if (result) {
//        return closelyAdaptiveResult;
//    }

//
//    Mat thresholdMat;
//    threshold(gray, thresholdMat, 50, 255, CV_THRESH_OTSU);
////    writeImage(thresholdMat,"thresholdMat-");
//    Result thresholdResult = decodePixels(thresholdMat);
//    result = thresholdResult.isValid();
//    if (result) {
//        return thresholdResult;
//    }
//
//    Mat huangMat;
//    gray.copyTo(huangMat);
//    int thresholdValue = GetHuangFuzzyThreshold(huangMat);
//    thresholdImage(huangMat, thresholdValue);
//    Result huangResult = decodePixels(huangMat, -1);
////    writeImage(huangMat,"huangMat-");
//    result = huangResult.isValid();
//    if (result) {
//        return huangResult;
//    }
//
    Result grayResult = decodePixels(gray, -1);
//    writeImage(gray,"gray-");
    result = grayResult.isValid();
    if (result) {
        return grayResult;
    }

    return Result(DecodeStatus::NotFound);

}

void hookZXingPhrases(int phrase,long p1,long p2,long p3){
//    ZXingHooker hooker;
//    hooker.hookHandler(phrase,p1,p2,p3);
}
Result ImageScheduler::hookZXing(const Mat &mat, int threshold){
    LOGE("detect by zxing");
    try {
        int width = mat.cols;
        int height = mat.rows;

        auto *pixels = new unsigned char[height * width];

        int index = 0;
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                if (threshold == -1) {
                    pixels[index++] = mat.at<unsigned char>(i, j);
                } else {
                    auto pValue = mat.at<unsigned char>(i, j);
                    if (pValue > threshold) {
                        pixels[index++] = 255;
                    } else {
                        pixels[index++] = 0;
                    }
                }
            }
        }

        auto binImage = BinaryBitmapFromBytesC1(pixels, 0, 0, width, height);
        ZXing::DecodeHints hints;
        auto reader = QRCode::Reader(hints);
        reader.setHookFunction(hookZXingPhrases);
        Result result = reader.decode(*binImage);
        delete[]pixels;

        if (result.isValid()) {
            return result;
        }
    } catch (const std::exception &e) {
        ThrowJavaException(env, e.what());
    }
    catch (...) {
        ThrowJavaException(env, "Unknown exception");
    }

    return Result(DecodeStatus::NotFound);
}

Result ImageScheduler::decodeZXing(const Mat &mat, int threshold) {
    LOGE("detect by zxing");
    try {
        int width = mat.cols;
        int height = mat.rows;

        auto *pixels = new unsigned char[height * width];

        int index = 0;
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                if (threshold == -1) {
                    pixels[index++] = mat.at<unsigned char>(i, j);
                } else {
                    auto pValue = mat.at<unsigned char>(i, j);
                    if (pValue > threshold) {
                        pixels[index++] = 255;
                    } else {
                        pixels[index++] = 0;
                    }
                }
            }
        }

        auto binImage = BinaryBitmapFromBytesC1(pixels, 0, 0, width, height);
        Result result = reader->read(*binImage);

        delete[]pixels;

        if (result.isValid()) {
            return result;
        }
    } catch (const std::exception &e) {
        ThrowJavaException(env, e.what());
    }
    catch (...) {
        ThrowJavaException(env, "Unknown exception");
    }

    return Result(DecodeStatus::NotFound);
}

Result ImageScheduler::decodeZBar(Mat &gray, int threshold) {
    LOGE("detect by zbar");

    int width = gray.cols;
    int height = gray.rows;

    thresholdImage(gray,threshold);
    ImageScanner scanner;
    scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
    const void *raw = gray.data;
    Image image(width, height, "Y800", raw, width * height);
    int n = scanner.scan(image);
    // 检测到二维码
    if (n > 0) {
        Image::SymbolIterator symbol = image.symbol_begin();
        image.set_data(nullptr, 0);
        if (symbol->get_type() == zbar_symbol_type_e::ZBAR_QRCODE) {
            Result result(DecodeStatus::NoError);
            result.setFormat(BarcodeFormat::QR_CODE);
            result.setText(ANSIToUnicode(symbol->get_data()));
            return result;
        }
    } else {
        image.set_data(nullptr, 0);
        return Result(DecodeStatus::NotFound);
    }
    return Result(DecodeStatus::NotFound);
}

