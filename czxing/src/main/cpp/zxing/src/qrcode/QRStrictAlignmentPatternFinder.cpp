//
// Created by uchia on 2019-12-18.
//

#include "qrcode/QRStrictAlignmentPatternFinder.h"
#include "qrcode/QRAlignmentPattern.h"
#include "BitMatrix.h"
#include "DecodeStatus.h"
#include "ZXContainerAlgorithms.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace ZXing {
namespace QRCode {


using StateCount = std::array<int, 3>;


static float CenterFromEnd(const StateCount& stateCount, int end)
{
    return static_cast<float>(end - stateCount[2]) - stateCount[1] / 2.0f;
}

static bool FoundPatternCross(const StateCount& stateCount, float moduleSize,float factor = 1)
{
    float maxVariance = moduleSize / 2.0f * factor;
    for (int i = 0; i < 3; i++) {
        if (std::fabs(moduleSize - stateCount[i]) >= maxVariance) {
            return false;
        }
    }
    return true;
}

static float CrossCheckVertical(const BitMatrix& image, int startI, int centerJ, int maxCount, int originalStateCountTotal, float moduleSize)
{
    int maxI = image.height();
    StateCount stateCount = {};

    // Start counting up from center
    int i = startI;
    while (i >= 0 && image.get(centerJ, i) && stateCount[1] <= maxCount) {
        stateCount[1]++;
        i--;
    }
    // If already too many modules in this state or ran off the edge:
    if (i < 0 || stateCount[1] > maxCount) {
        return std::numeric_limits<float>::quiet_NaN();
    }
    while (i >= 0 && !image.get(centerJ, i) && stateCount[0] <= maxCount) {
        stateCount[0]++;
        i--;
    }
    if (stateCount[0] > maxCount) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    // Now also count down from center
    i = startI + 1;
    while (i < maxI && image.get(centerJ, i) && stateCount[1] <= maxCount) {
        stateCount[1]++;
        i++;
    }
    if (i == maxI || stateCount[1] > maxCount) {
        return std::numeric_limits<float>::quiet_NaN();
    }
    while (i < maxI && !image.get(centerJ, i) && stateCount[2] <= maxCount) {
        stateCount[2]++;
        i++;
    }
    if (stateCount[2] > maxCount) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    int stateCountTotal = Accumulate(stateCount, 0);
    if (5 * std::abs(stateCountTotal - originalStateCountTotal) >= 2 * originalStateCountTotal) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    return FoundPatternCross(stateCount, moduleSize) ? CenterFromEnd(stateCount, i) : std::numeric_limits<float>::quiet_NaN();
}

static bool CheckNotOutOfRange(
        int horizontalStart, int horizontalEnd,
        int verticalStart, int verticalEnd,
        int currentH, int currentV){

    if (currentH < horizontalStart || currentH > horizontalEnd){
        return false;
    }

    if (currentV < verticalStart || currentV > verticalEnd){
        return false;
    }
    return true;
}

static bool CollectPixelCount(const BitMatrix& image, int centerI, int centerJ,
        int width,int height,int maxCount,
        int horizontalStep,int verticalStep,
        int originalStateCountTotal, float moduleSize) {
    int horizontalStart = centerJ - width / 2;
    int verticalStart = centerI - height / 2;
    int horizontalEnd = centerJ + width / 2;
    int verticalEnd = centerI + height / 2;

    if (horizontalStart < 0 || verticalStart < 0 || horizontalEnd >= image.width() || verticalEnd >= image.height()) {
        return false;
    }

    auto diagonalLine = std::array<int, 3>();

    int t = centerI;
    int l = centerJ;
    while (CheckNotOutOfRange(horizontalStart, horizontalEnd, verticalStart, verticalEnd, l, t) && image.get(l, t) && diagonalLine[1] <= maxCount) {
        diagonalLine[1]++;
        t -= verticalStep;
        l -= horizontalStep;
    }

    if (!CheckNotOutOfRange(horizontalStart, horizontalEnd, verticalStart, verticalEnd, l, t) || diagonalLine[1] > maxCount) {
        return false;
    }

    while (CheckNotOutOfRange(horizontalStart, horizontalEnd, verticalStart, verticalEnd, l, t) && !image.get(l, t) && diagonalLine[0] <= maxCount) {
        diagonalLine[0]++;
        t -= verticalStep;
        l -= horizontalStep;
    }
    if (diagonalLine[0] > maxCount) {
        return false;
    }

    t = centerI + 1;
    l = centerJ + 1;
    while (CheckNotOutOfRange(horizontalStart, horizontalEnd, verticalStart, verticalEnd, l, t) && image.get(l, t) && diagonalLine[1] <= maxCount) {
        diagonalLine[1]++;
        t += verticalStep;
        l += horizontalStep;
    }

    if (!CheckNotOutOfRange(horizontalStart, horizontalEnd, verticalStart, verticalEnd, l, t) || diagonalLine[1] > maxCount) {
        return false;
    }

    while (CheckNotOutOfRange(horizontalStart, horizontalEnd, verticalStart, verticalEnd, l, t) && !image.get(l, t) && diagonalLine[2] <= maxCount) {
        diagonalLine[2]++;
        t += verticalStep;
        l += horizontalStep;
    }
    if (diagonalLine[2] > maxCount) {
        return false;
    }

    int stateCountTotal = Accumulate(diagonalLine, 0);
    if (5 * std::abs(stateCountTotal - originalStateCountTotal) >= 2 * originalStateCountTotal) {
        return false;
    }

    return FoundPatternCross(diagonalLine, moduleSize, 1.5f);
}

static bool CrossCheckDualDiagonal(const BitMatrix& image, int centerI, int centerJ,
        int width,int height,int maxCount,
        int originalStateCountTotal, float moduleSize){
    auto leftDiagonalResult = CollectPixelCount(image,centerI,centerJ,
            width,height,maxCount,
            1,1,originalStateCountTotal,moduleSize);
    if (!leftDiagonalResult){
        return false;
    }

    auto rightDiagonalResult = CollectPixelCount(image,centerI,centerJ,
            width,height,maxCount,
            -1,1,originalStateCountTotal,moduleSize);

    return rightDiagonalResult;
}


static AlignmentPattern
HandlePossibleCenter(const BitMatrix& image, const StateCount& stateCount, int i, int j,int width,int height, float moduleSize, std::vector<AlignmentPattern>& possibleCenters)
{
    int stateCountTotal = Accumulate(stateCount, 0);
    float centerJ = CenterFromEnd(stateCount, j);
    float centerI = CrossCheckVertical(image, i, static_cast<int>(centerJ), 2 * stateCount[1], stateCountTotal, moduleSize);
    if (!CrossCheckDualDiagonal(image,centerI,centerJ,width,height,2 * moduleSize,stateCountTotal,moduleSize)){
        return {};
    }
    if (!std::isnan(centerI)) {
        float estimatedModuleSize = stateCountTotal / 3.0f;
        for (const AlignmentPattern& center : possibleCenters) {
            // Look for about the same center and module size:
            if (center.aboutEquals(estimatedModuleSize, centerI, centerJ)) {
                return center.combineEstimate(centerI, centerJ, estimatedModuleSize);
            }
        }
        // Hadn't found this before; save it
        possibleCenters.emplace_back(centerJ, centerI, estimatedModuleSize);
    }
    return {};
}

AlignmentPattern
StrictAlignmentPatternFinder::Find(const BitMatrix& image, int startX, int startY, int width, int height, float moduleSize)
{
    int maxJ = startX + width;
    int middleI = startY + (height / 2);
    std::vector<AlignmentPattern> possibleCenters;
    possibleCenters.reserve(5);

    // We are looking for black/white/black modules in 1:1:1 ratio;
    // this tracks the number of black/white/black modules seen so far
    for (int iGen = 0; iGen < height; iGen++) {
        // Search from middle outwards
        StateCount stateCount = {};
        int i = middleI + ((iGen & 0x01) == 0 ? (iGen + 1) / 2 : -((iGen + 1) / 2));
        int j = startX;
        // Burn off leading white pixels before anything else; if we start in the middle of
        // a white run, it doesn't make sense to count its length, since we don't know if the
        // white run continued to the left of the start point
        while (j < maxJ && !image.get(j, i)) {
            j++;
        }
        int currentState = 0;
        while (j < maxJ) {
            if (image.get(j, i)) {
                // Black pixel
                if (currentState == 1) { // Counting black pixels
                    stateCount[1]++;
                }
                else { // Counting white pixels
                    if (currentState == 2) { // A winner?
                        if (FoundPatternCross(stateCount, moduleSize)) { // Yes
                            auto result = HandlePossibleCenter(image, stateCount, i, j,width,height, moduleSize, possibleCenters);
                            if (result.isValid())
                                return result;
                        }
                        stateCount[0] = stateCount[2];
                        stateCount[1] = 1;
                        stateCount[2] = 0;
                        currentState = 1;
                    }
                    else {
                        stateCount[++currentState]++;
                    }
                }
            }
            else { // White pixel
                if (currentState == 1) { // Counting black pixels
                    currentState++;
                }
                stateCount[currentState]++;
            }
            j++;
        }
        if (FoundPatternCross(stateCount, moduleSize)) {
            auto result = HandlePossibleCenter(image, stateCount, i, maxJ,width,height, moduleSize, possibleCenters);
            if (result.isValid())
                return result;
        }

    }

    // Hmm, nothing we saw was observed and confirmed twice. If we had
    // any guess at all, return it.
    if (!possibleCenters.empty()) {
        return possibleCenters.front();
    }

    return {};
}

} // QRCode
} // ZXing
