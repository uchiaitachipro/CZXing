//
// Created by Devilsen on 2019-08-09.
//

#ifndef CZXING_IMAGESCHEDULER_H
#define CZXING_IMAGESCHEDULER_H


#include <jni.h>
#include <opencv2/core/mat.hpp>
#include <src/MultiFormatReader.h>
#include <src/BinaryBitmap.h>
#include "Result.h"
#include "JavaCallHelper.h"
#include "QRCodeRecognizer.h"
#include "safe_queue.h"
#include "QRCodeFinder.h"
#include <mutex>
#include "zbar/include/zbar.h"
#include "TimeProfiler.h"

using namespace cv;
using namespace ZXing;
using namespace zbar;

typedef struct FrameData {
    jbyte *bytes;
    int left;
    int top;
    int cropWidth;
    int cropHeight;
    int rowWidth;
    int rowHeight;
} FrameData;

class ImageScheduler {

public:
    enum DecodeStrategy {
        STRATEGY_RAW_PICTURE = 1,
        STRATEGY_THRESHOLD = 2,
        STRATEGY_ADAPTIVE_THRESHOLD_CLOSELY = 4,
        STRATEGY_COLOR_EXTRACT = 8,
        STRATEGY_HUANG_FUZZY = 16,
        STRATEGY_ADAPTIVE_THRESHOLD_REMOTELY= 32
    };

    enum DetectorType{
        ZXING = 0,
        ZBAR = 1,
        ALL = 2
    };

    ImageScheduler(JNIEnv *env, MultiFormatReader *_reader, JavaCallHelper *javaCallHelper);

    ~ImageScheduler();

    void prepare();

    void start();

    void stop();

    void
    process(jbyte *bytes, int left, int top, int width, int height, int rowWidth, int rowHeight,int strategyIndex);

    Result readBitmap(const cv::Mat &mat, int left, int top, int width, int height);

    void setStrategies(vector<int> &strategies) {
        _strategies.assign(strategies.begin(), strategies.end());
    }

    void setApplyAllStrategie(bool r) {
        isApplyAllStrategies = r;
    }

    void applyAllStrategies(bool result){
        isApplyAllStrategies  = result;
    }

    void setDecoderType(int type){
        detectType = type;
    }

private:
    JNIEnv *env;
    MultiFormatReader *reader;
    JavaCallHelper *javaCallHelper;
    std::atomic<bool> isProcessing{};
    std::atomic<bool> stopProcessing{};
    std::atomic<bool> abortTask{};
    vector<int> _strategies;
    QRCodeRecognizer *qrCodeRecognizer;
    SafeQueue<FrameData> frameQueue;
    QRCodeFinder qrCodeFinder;
    pthread_t prepareThread{};
    double cameraLight{};
    bool isApplyAllStrategies = false;
    int detectType = DetectorType::ZXING;
    int currentStrategyIndex = 0;

    void initThreadPool();

    void preTreatMat(const FrameData &frameData);

    void applyStrategy(Mat &mat);

    Result decodePixels(Mat &mat,int threshold = -1);

    Result decodeZBar(Mat &gray, int threshold);

    Result decodeZXing(const Mat &mat, int threshold);

    Result decodeGrayPixels(Mat &gray);

    Result decodeThresholdPixels(Mat &gray);

    Result decodeAdaptivePixels(Mat &gray,int adaptiveMethod,int blockSize,int delta);

    void recognizerQrCode(const Mat &mat);

    void filterColorInImage(const Mat &raw, Mat &outImage);

    bool analysisBrightness(const Mat &gray);

};

#endif //CZXING_IMAGESCHEDULER_H
