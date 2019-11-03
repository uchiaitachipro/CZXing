//
// Created by uchia on 2019-11-02.
//

#ifndef CZXING_THRESHOLDTOOL_H
#define CZXING_THRESHOLDTOOL_H

#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>
#include <limits>
#include "math.h"

using namespace cv;


int GetHuangFuzzyThreshold(const cv::Mat &image) {
    int count = 256;
    vector<int> histGram(count);
    for (auto w = 0; w < image.rows; ++w) {
        for (auto h = 0; h < image.cols; ++h) {
            int index = image.at<uchar>(w, h);
            histGram[index] += 1;
        }
    }


    long x = 0, y = 0;
    long first = 0, last = 0;
    int threshold = -1;
    double bestEntropy = std::numeric_limits<double>::max(), entropy;
    //   找到第一个和最后一个非0的色阶值
    for (first = 0; first < count && histGram[first] == 0; first++);
    for (last = count - 1; last > first && histGram[last] == 0; last--);
    if (first == last) return first;                // 图像中只有一个颜色
    if (first + 1 == last) return first;            // 图像中只有二个颜色

    // 计算累计直方图以及对应的带权重的累计直方图
    std::vector<long> s(last + 1);
    std::vector<long> w(last + 1);  // 对于特大图，此数组的保存数据可能会超出int的表示范围，可以考虑用long类型来代替
    s[0] = histGram[0];
    for (y = first > 1 ? first : 1; y <= last; y++) {
        s[y] = s[y - 1] + histGram[y];
        w[y] = w[y - 1] + y * histGram[y];
    }

    // 建立公式（4）及（6）所用的查找表
    long sumCount = last + 1 - first;
    std::vector<double> smu(sumCount);
    for (y = 1; y < sumCount; y++) {
        float mu = 1 / (1 + (float) y / (last - first));               // 公式（4）
        auto logMu = log(mu);
        auto logReverseMu = log(1 - mu);
        smu[y] = -mu * logMu - (1 - mu) * logReverseMu;      // 公式（6）
    }

    // 迭代计算最佳阈值
    for (y = first; y <= last; y++) {
        entropy = 0;
        int mu = (int) round(((double) w[y]) / s[y]);             // 公式17
        for (x = first; x <= y; x++)
            entropy += smu[abs(x - mu)] * histGram[x];
        mu = (int) round(((double) (w[last] - w[y])) / (s[last] - s[y]));  // 公式18
        for (x = y + 1; x <= last; x++)
            entropy += smu[abs(x - mu)] * histGram[x];       // 公式8
        if (bestEntropy > entropy) {
            bestEntropy = entropy;      // 取最小熵处为最佳阈值
            threshold = y;
        }
    }

    return threshold;
}

void thresholdImage(cv::Mat &image, int threshold) {
    if (threshold <= 0) {
        return;
    }
    int width = image.cols;
    int height = image.rows;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            auto pValue = image.at<unsigned char>(i, j);
            if (pValue > threshold) {
                image.at<unsigned char>(i, j) = 255;
            } else {
                image.at<unsigned char>(i, j) = 0;
            }

        }
    }
}


#endif //CZXING_THRESHOLDTOOL_H
