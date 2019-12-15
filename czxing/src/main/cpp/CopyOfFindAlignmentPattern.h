//
// Created by uchia on 2019-12-15.
//

#ifndef CZXING_COPYOFFINDALIGNMENTPATTERN_H
#define CZXING_COPYOFFINDALIGNMENTPATTERN_H

#include <src/qrcode/QRAlignmentPattern.h>
#include <src/BitMatrix.h>
#include <src/ZXContainerAlgorithms.h>
#include <array>

using namespace std;
using namespace ZXing;
using namespace QRCode;
using StateCount = std::array<int, 3>;

float copyCenterFromEnd(const StateCount& stateCount, int end)
{
    return static_cast<float>(end - stateCount[2]) - stateCount[1] / 2.0f;
}

bool copyFoundPatternCross(const StateCount& stateCount, float moduleSize)
{
    float maxVariance = moduleSize / 2.0f;
    for (int i = 0; i < 3; i++) {
        if (std::fabs(moduleSize - stateCount[i]) >= maxVariance) {
            return false;
        }
    }
    return true;
}

float copyCrossCheckVertical(const BitMatrix& image, int startI, int centerJ, int maxCount, int originalStateCountTotal, float moduleSize)
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

    return copyFoundPatternCross(stateCount, moduleSize) ? copyCenterFromEnd(stateCount, i) : std::numeric_limits<float>::quiet_NaN();
}

AlignmentPattern
copyHandlePossibleCenter(const BitMatrix& image, const StateCount& stateCount, int i, int j, float moduleSize, std::vector<AlignmentPattern>& possibleCenters)
{
    int stateCountTotal = Accumulate(stateCount, 0);
    float centerJ = copyCenterFromEnd(stateCount, j);
    float centerI = copyCrossCheckVertical(image, i, static_cast<int>(centerJ), 2 * stateCount[1], stateCountTotal, moduleSize);
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

void findAlignment(const BitMatrix& image, int startX, int startY, int width, int height, float moduleSize)
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
                        if (copyFoundPatternCross(stateCount, moduleSize)) { // Yes
                            auto result = copyHandlePossibleCenter(image, stateCount, i, j, moduleSize, possibleCenters);
                            if (result.isValid()){
                                return;
                            }
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

    }
    return;
}

#endif //CZXING_COPYOFFINDALIGNMENTPATTERN_H
