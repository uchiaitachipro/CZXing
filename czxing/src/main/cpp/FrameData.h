//
// Created by uchia on 2020/1/18.
//

#ifndef CZXING_FRAMEDATA_H
#define CZXING_FRAMEDATA_H

#include <jni.h>

typedef struct FrameData {
    jbyte *bytes;
    int left;
    int top;
    int cropWidth;
    int cropHeight;
    int rowWidth;
    int rowHeight;
} FrameData;
#endif //CZXING_FRAMEDATA_H
