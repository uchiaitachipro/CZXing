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

using namespace cv;
using namespace ZXing;

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
        STRATEGY_ADAPTIVE_THRESHOLD = 4,
        STRATEGY_COLOR_EXTRACT = 8,
        STRATEGY_LOCATE_QR_CODE = 16
    };

    ImageScheduler(JNIEnv *env, MultiFormatReader *_reader, JavaCallHelper *javaCallHelper);

    ~ImageScheduler();

    void prepare();

    void start();

    void stop();

    void
    process(jbyte *bytes, int left, int top, int width, int height, int rowWidth, int rowHeight);

    Result readBitmap(jobject bitmap, int left, int top, int width, int height);

    void setStrategies(vector<int> &strategies) {
        _strategies.assign(strategies.begin(), strategies.end());
    }

    void setApplyAllStrategie(bool r) {
        isApplyAllStrategies = r;
    }

private:
    JNIEnv *env;
    MultiFormatReader *reader;
    JavaCallHelper *javaCallHelper;
    std::atomic<bool> isProcessing{};
    std::atomic<bool> stopProcessing{};
    vector<int> _strategies;
    QRCodeRecognizer *qrCodeRecognizer;
    SafeQueue<FrameData> frameQueue;
    QRCodeFinder qrCodeFinder;
    pthread_t prepareThread{};
    double cameraLight{};
    bool isApplyAllStrategies = true;
    int currentStrategyIndex = 0;

    void preTreatMat(const FrameData &frameData);

    void applyStrategy(const Mat &mat);

    Result decodePixels(const Mat &mat);

    bool decodeGrayPixels(const Mat &gray);

    bool decodeThresholdPixels(const Mat &gray);

    bool decodeAdaptivePixels(const Mat &gray);

    void recognizerQrCode(const Mat &mat);

    void filterColorInImage(const Mat &raw, Mat &outImage);

    bool analysisBrightness(const Mat &gray);

};

#endif //CZXING_IMAGESCHEDULER_H
