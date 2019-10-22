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
//    initThreadPool();
}

ImageScheduler::~ImageScheduler() {
    DELETE(env);
    DELETE(reader);
    DELETE(javaCallHelper);
    DELETE(qrCodeRecognizer);
    DELETE(pool);
    frameQueue.clear();
    queue.clear();
    delete &isProcessing;
    delete &stopProcessing;
    delete &cameraLight;
    delete &prepareThread;
}


void ImageScheduler::initThreadPool() {
    if (pool == NULL) {
        pool = new ThreadPool(threadPoolCount);
    }
    for (int i = 0; i < threadPoolCount; i++) {
        pool->enqueue([&] {
            while (true) {
                if (abortTask.load()) {
                    break;
                }
                FrameData frameData;
                int ret = frameQueue.deQueue(frameData);
                if (ret){
                    preTreatMat(frameData);
                }
            }
        });
    }
}

void *prepareMethod(void *arg) {
    auto scheduler = static_cast<ImageScheduler *>(arg);
    scheduler->start();
    return nullptr;
}

void ImageScheduler::prepare() {
    pthread_create(&prepareThread, nullptr, prepareMethod, this);
}
//
//void ImageScheduler::prepare() {
////    if (pool == NULL){
////        pool = new ThreadPool(threadPoolCount);
////    }
////    for (int i = 0; i < threadPoolCount; i++){
////        pool->enqueue([&]{
////            while(true){
////                if(abortTask.load()){
////                    break;
////                }
////                FrameData frameData = queue.take();
////                preTreatMat(frameData);
////            }
////        });
////    }
//}

void ImageScheduler::start() {
    stopProcessing.store(false);
    isProcessing.store(false);
    frameQueue.setWork(1);
    currentStrategyIndex = 0;
    while (true) {
        if (stopProcessing.load()) {
            break;
        }

        if (isProcessing.load()) {
            continue;
        }

        FrameData frameData;
        int ret = frameQueue.deQueue(frameData);
        if (ret) {
            isProcessing.store(true);
            preTreatMat(frameData);
            isProcessing.store(false);
        }
    }
}

void ImageScheduler::stop() {
    currentStrategyIndex = 0;
    isProcessing.store(false);
    stopProcessing.store(true);
    abortTask.store(true);
    frameQueue.setWork(0);
    frameQueue.clear();
}

void
ImageScheduler::process(jbyte *bytes, int left, int top, int cropWidth, int cropHeight,
                        int rowWidth,
                        int rowHeight) {
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

void ImageScheduler::applyStrategy(const Mat &mat) {

    int startIndex = isApplyAllStrategies ? 0 : currentStrategyIndex;
    bool result = false;

    for (int i = startIndex; i < _strategies.size(); ++i) {

        switch (_strategies[i]) {
            case DecodeStrategy::STRATEGY_RAW_PICTURE:
                result = decodeGrayPixels(mat);
                break;
            case DecodeStrategy::STRATEGY_THRESHOLD:
                result = decodeThresholdPixels(mat);
                break;
            case DecodeStrategy::STRATEGY_ADAPTIVE_THRESHOLD_CLOSELY:
                result = decodeAdaptivePixels(mat,ADAPTIVE_THRESH_MEAN_C,55,3);
                break;
            case DecodeStrategy::STRATEGY_ADAPTIVE_THRESHOLD_ROMOTELY:
                result = decodeAdaptivePixels(mat,ADAPTIVE_THRESH_GAUSSIAN_C,25,5);
                break;
            case DecodeStrategy::STRATEGY_COLOR_EXTRACT:
                break;
            default:
                break;
        }
        LOGE("current strategy index : %d", currentStrategyIndex);
        if (result || !isApplyAllStrategies) {
            break;
        }
    }
    if (!result) {
        recognizerQrCode(mat);
    }
    currentStrategyIndex = (currentStrategyIndex + 1) % (_strategies.size());
}

void ImageScheduler::filterColorInImage(const Mat &src, Mat &outImage) {
    Mat rgbMat;
//    cvtColor(src, rgbMat, COLOR_YUV2BGR_NV21);
    Mat hsv;
    cvtColor(rgbMat, hsv, CV_BGR2HSV);

    for (int w = 0; w < rgbMat.rows; w++) {
        for (int h = 0; h < rgbMat.cols; h++) {
            auto pixel = rgbMat.at<Vec3b>(w, h);

            auto b = pixel[0];
            auto g = pixel[1];

            if (b < 100 || g < 100) {
                continue;
            }

            rgbMat.at<Vec3b>(w, h)[0] = 255;
            rgbMat.at<Vec3b>(w, h)[1] = 255;
            rgbMat.at<Vec3b>(w, h)[2] = 255;

//           if (pixel[1] >= 140 && pixel[2] >= 130 && pixel[0] >= 87 && pixel[0] <= 124){
//               H += pixel[0];
//               s += pixel[1];
//               v += pixel[2];
//               ++count;
//               hsv.at<Vec3b>(w,h)[0] = 180;
//               hsv.at<Vec3b>(w,h)[1] = 30;
//               hsv.at<Vec3b>(w,h)[2] = 255;
//           }

        }
    }

//    LOGE("h channel average: %lf", (H / count));
//    LOGE("s channel average: %lf", (s / count));
//    LOGE("v channel average: %lf", (v / count));
//    cvtColor(hsv,rgbMat,CV_HSV2BGR);
    Mat th;
    cvtColor(rgbMat, th, COLOR_BGR2GRAY);
    threshold(th, th, 50, 255, CV_THRESH_OTSU);
    writeImage(th, "color");
}

bool ImageScheduler::decodeGrayPixels(const Mat &gray) {
    LOGE("start GrayPixels...");

    Mat mat;
    rotate(gray, mat, ROTATE_90_CLOCKWISE);
    Result result = decodePixels(gray);
    if (result.isValid()) {
//        writeImage(gray,"gray-");
//        qrCodeFinder.locateQRCode(mat, 200, 5, false);
        javaCallHelper->onResult(result,cameraLight);
    }
    return result.isValid();
}

bool ImageScheduler::decodeThresholdPixels(const Mat &gray) {
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
    if (result.isValid()) {
        javaCallHelper->onResult(result,cameraLight);
//        Rect rect =  qrCodeFinder.locateQRCode(mat, 200, 5, false);
//        writeImage(mat, std::string("threshold-"));
    }
//            writeImage(mat, std::string("threshold-"));
    return result.isValid();
}

bool ImageScheduler::decodeAdaptivePixels(const Mat &gray,int adaptiveMethod,int blockSize,int delta) {
    LOGE("start AdaptivePixels...");

    Mat mat;
    rotate(gray, mat, ROTATE_90_COUNTERCLOCKWISE);

    // 降低图片亮度
    Mat lightMat;
    mat.convertTo(lightMat, -1, 1.0, -60);

    adaptiveThreshold(lightMat, lightMat, 255, adaptiveMethod,
                      THRESH_BINARY, blockSize, delta);
    Result result = decodePixels(lightMat);
    if (result.isValid()) {
        javaCallHelper->onResult(result,cameraLight);
//        Rect rect =  qrCodeFinder.locateQRCode(mat, 200, 5, false);
//        writeImage(mat, std::string("adaptive-threshold-ROI-"));
    }
//    writeImage(lightMat, std::string("adaptive-threshold-"));
    return result.isValid();
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

    javaCallHelper->onResult(result,cameraLight);

    LOGE("end recognizerQrCode...");

}

Result ImageScheduler::decodePixels(const Mat &mat) {

    try {
        int width = mat.cols;
        int height = mat.rows;

        auto *pixels = new unsigned char[height * width];

        int index = 0;
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                pixels[index++] = mat.at<unsigned char>(i, j);
            }
        }

        auto binImage = BinaryBitmapFromBytesC1(pixels, 0, 0, width, height);
        Result result = reader->read(*binImage);

        delete[]pixels;

        return result;
    } catch (const std::exception &e) {
        ThrowJavaException(env, e.what());
    }
    catch (...) {
        ThrowJavaException(env, "Unknown exception");
    }

    return Result(DecodeStatus::NotFound);
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

Result ImageScheduler::readBitmap(jobject bitmap, int left, int top, int width, int height) {
    auto binImage = BinaryBitmapFromJavaBitmap(env, bitmap, left, top, width, height);
    if (!binImage) {
        LOGE("create binary bitmap fail");
        return Result(DecodeStatus::NotFound);
    }
    return reader->read(*binImage);
}

