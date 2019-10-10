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
    ImageScheduler(JNIEnv *env, MultiFormatReader *_reader, JavaCallHelper *javaCallHelper);

    ~ImageScheduler();

    void
    process(jbyte *bytes, int left, int top, int width, int height, int rowWidth,
            int rowHeight);

    void prepare();

    void start();

    void stop();

    void preTreatMat(const FrameData& frameData);

    void decodeGrayPixels(const Mat& gray);

    void decodeThresholdPixels(const Mat& gray);

    void decodeAdaptivePixels(const Mat& gray);

    void decodeSingleChannel(const Mat& mat);

    Result readBitmap(jobject bitmap, int left, int top, int width,int height);

private:
    JNIEnv *env;
    MultiFormatReader *reader;
    JavaCallHelper *javaCallHelper;
    std::atomic<bool> isProcessing{};
    std::atomic<bool> stopProcessing{};
    double cameraLight{};
    QRCodeRecognizer *qrCodeRecognizer;
    SafeQueue<FrameData> frameQueue;
    QRCodeFinder qrCodeFinder;


    pthread_t prepareThread{};

    Result decodePixels(Mat mat);

    void recognizerQrCode(const Mat& mat);

    Result *analyzeResult();

    bool  analysisBrightness(const Mat& gray);

};

#endif //CZXING_IMAGESCHEDULER_H
