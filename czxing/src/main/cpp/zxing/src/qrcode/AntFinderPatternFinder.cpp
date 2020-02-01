//
// Created by uchiachen on 2020-01-22.
//

#include "AntFinderPatternFinder.h"
#include "qrcode/QRFinderPatternInfo.h"
#include "BitMatrix.h"
#include "DecodeHints.h"
#include "DecodeStatus.h"
#include "ZXContainerAlgorithms.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <algorithm>
namespace ZXing {
namespace QRCode {

static const int CENTER_QUORUM = 2;
static const int MIN_SKIP = 3; // 1 pixel/module times 3 modules/center
static const int MAX_MODULES = 97; // support up to version 20 for mobile clients

using StateCount = AntFinderPatternFinder::StateCount;

static float CrossProductZ(const ResultPoint& a, const ResultPoint& b, const ResultPoint& c)
{
    return (c.x() - b.x())*(a.y() - b.y()) - (c.y() - b.y())*(a.x() - b.x());
}

static void OrderBestPatterns(std::vector<FinderPattern>& patterns)
{
    assert(patterns.size() == 3);

    auto &p0 = patterns[0], &p1 = patterns[1], &p2 = patterns[2];

    // Find distances between pattern centers
    float zeroOneDistance = ResultPoint::Distance(p0, p1);
    float oneTwoDistance = ResultPoint::Distance(p1, p2);
    float zeroTwoDistance = ResultPoint::Distance(p0, p2);

    // Assume one closest to other two is B; A and C will just be guesses at first
    if (oneTwoDistance >= zeroOneDistance && oneTwoDistance >= zeroTwoDistance)
        std::swap(p0, p1);
    else if (zeroTwoDistance >= oneTwoDistance && zeroTwoDistance >= zeroOneDistance)
        ; // do nothing, the order is correct
    else
        std::swap(p1, p2);

    // Use cross product to figure out whether A and C are correct or flipped.
    // This asks whether BC x BA has a positive z component, which is the arrangement
    // we want for A, B, C. If it's negative, then we've got it flipped around and
    // should swap A and C.
    if (CrossProductZ(p0, p1, p2) < 0.0f) {
        std::swap(p0, p2);
    }
}

struct EstimatedModuleComparator
{
    bool operator()(const FinderPattern& center1, const FinderPattern& center2) const {
        return center1.estimatedModuleSize() < center2.estimatedModuleSize();
    }
};

static std::vector<FinderPattern> SelectBestPatterns(std::vector<FinderPattern> possibleCenters)
{
    std::vector<FinderPattern> bestPatterns;
    int nbPossibleCenters = static_cast<int>(possibleCenters.size());
    if (nbPossibleCenters < 3) {
        // Couldn't find enough finder patterns
        return bestPatterns;
    }

    std::sort(possibleCenters.begin(), possibleCenters.end(), EstimatedModuleComparator());

    double distortion = std::numeric_limits<double>::max();
    std::array<double, 3> squares;

    for (int i = 0; i < nbPossibleCenters - 2; i++) {
        auto& fpi = possibleCenters[i];
        float minModuleSize = fpi.estimatedModuleSize();

        for (int j = i + 1; j < nbPossibleCenters - 1; j++) {
            auto& fpj = possibleCenters[j];
            double squares0 = ResultPoint::SquaredDistance(fpi, fpj);

            for (int k = j + 1; k < nbPossibleCenters; k++) {
                auto& fpk = possibleCenters[k];
                float maxModuleSize = fpk.estimatedModuleSize();
                if (maxModuleSize > minModuleSize * 1.4f) {
                    // module size is not similar
                    continue;
                }

                squares[0] = squares0;
                squares[1] = ResultPoint::SquaredDistance(fpj, fpk);
                squares[2] = ResultPoint::SquaredDistance(fpi, fpk);
                std::sort(squares.begin(), squares.end());

                // a^2 + b^2 = c^2 (Pythagorean theorem), and a = b (isosceles triangle).
                // Since any right triangle satisfies the formula c^2 - b^2 - a^2 = 0,
                // we need to check both two equal sides separately.
                // The value of |c^2 - 2 * b^2| + |c^2 - 2 * a^2| increases as dissimilarity
                // from isosceles right triangle.
                double d = std::abs(squares[2] - 2 * squares[1]) + std::abs(squares[2] - 2 * squares[0]);
                if (d < distortion) {
                    distortion = d;
                    bestPatterns.resize(3);
                    bestPatterns[0] = fpi;
                    bestPatterns[1] = fpj;
                    bestPatterns[2] = fpk;
                }
            }
        }
    }

    return bestPatterns;
}

static bool HaveMultiplyConfirmedCenters(const std::vector<FinderPattern>& possibleCenters)
{
    int confirmedCount = 0;
    float totalModuleSize = 0.0f;
    //int max = possibleCenters.size();
    for (const FinderPattern& pattern : possibleCenters) {
        if (pattern.count() >= CENTER_QUORUM) {
            confirmedCount++;
            totalModuleSize += pattern.estimatedModuleSize();
        }
    }
    if (confirmedCount < 3) {
        return false;
    }
    // OK, we have at least 3 confirmed centers, but, it's possible that one is a "false positive"
    // and that we need to keep looking. We detect this by asking if the estimated module sizes
        // vary too much. We arbitrarily say that when the total deviation from average exceeds
    // 5% of the total module size estimates, it's too much.
    float average = totalModuleSize / static_cast<float>(possibleCenters.size());
    float totalDeviation = 0.0f;
    for (const FinderPattern& pattern : possibleCenters) {
        totalDeviation += std::abs(pattern.estimatedModuleSize() - average);
    }
    return totalDeviation <= 0.05f * totalModuleSize;
}

static int FindRowSkip(const std::vector<FinderPattern>& possibleCenters, bool& hasSkipped)
{
    if (possibleCenters.size() <= 1) {
        return 0;
    }

    const ResultPoint* firstConfirmedCenter = nullptr;
    for (const FinderPattern& center : possibleCenters) {
        if (center.count() >= CENTER_QUORUM) {
            if (firstConfirmedCenter == nullptr) {
                firstConfirmedCenter = &center;
            }
            else {
                // We have two confirmed centers
                // How far down can we skip before resuming looking for the next
                // pattern? In the worst case, only the difference between the
                // difference in the x / y coordinates of the two centers.
                // This is the case where you find top left last.
                hasSkipped = true;
                return static_cast<int>(std::abs(firstConfirmedCenter->x() - center.x()) - std::abs(firstConfirmedCenter->y() - center.y())) / 2;
            }
        }
    }
    return 0;
}

FinderPatternInfo AntFinderPatternFinder::Find(const BitMatrix& image, bool tryHarder,HookFunction f)
{
    int maxI = image.height();
    int maxJ = image.width();
    // We are looking for black/white/black/white/black modules in
    // 1:1:3:1:1 ratio; this tracks the number of such modules seen so far

    // Let's assume that the maximum version QR Code we support takes up 1/4 the height of the
    // image, and then account for the center being 3 modules in size. This gives the smallest
    // number of pixels the center could be, so skip this often. When trying harder, look for all
    // QR versions regardless of how dense they are.
    int iSkip = (3 * maxI) / (4 * MAX_MODULES);
    if (iSkip < MIN_SKIP || tryHarder) {
        iSkip = MIN_SKIP;
    }

    bool hasSkipped = false;
    std::vector<FinderPattern> possibleCenters;

    bool done = false;
    for (int i = iSkip - 1; i < maxI && !done; i += iSkip) {
        // Get a row of black/white values
        StateCount stateCount = {};
        size_t currentState = 0;
        for (int j = 0; j < maxJ; j++) {
            if (image.get(j, i)) {
                // Black pixel
                if ((currentState & 1) == 1) { // Counting white pixels
                    currentState++;
                }
                stateCount[currentState]++;
            }
            else { // White pixel
                if ((currentState & 1) == 0) { // Counting black pixels
                    if (currentState == 2) { // A winner?
                        if (FoundPatternCross(stateCount)) { // Yes
                            bool confirmed = HandlePossibleCenter(image, stateCount, i, j, possibleCenters,f);
                            if (confirmed) {
                                // Start examining every other line. Checking each line turned out to be too
                                // expensive and didn't improve performance.
                                iSkip = 2;
                                if (hasSkipped) {
                                    done = HaveMultiplyConfirmedCenters(possibleCenters);
                                }
                                else {
                                    int rowSkip = FindRowSkip(possibleCenters, hasSkipped);
                                    if (rowSkip > stateCount[1]) {
                                        // Skip rows between row of lower confirmed center
                                        // and top of presumed third confirmed center
                                        // but back up a bit to get a full chance of detecting
                                        // it, entire width of center of finder pattern

                                        // Skip by rowSkip, but back off by stateCount[2] (size of last center
                                        // of pattern we saw) to be conservative, and also back off by iSkip which
                                        // is about to be re-added
                                        i += rowSkip - stateCount[1] - iSkip;
                                        j = maxJ - 1;
                                    }
                                }
                            }
                            else {
                                stateCount[0] = stateCount[2];
                                stateCount[1] = 1;
                                stateCount[2] = 0;
                                currentState = 1;
                                continue;
                            }
                            // Clear state to start looking again
                            currentState = 0;
                            stateCount.fill(0);
                        }
                        else { // No, shift counts back by two
                            stateCount[0] = stateCount[2];
                            stateCount[1] = 1;
                            stateCount[2] = 0;
                            currentState = 1;
                        }
                    }
                    else {
                        stateCount[++currentState]++;
                    }
                }
                else { // Counting white pixels
                    stateCount[currentState]++;
                }
            }
        }
        if (AntFinderPatternFinder::FoundPatternCross(stateCount)) {
            bool confirmed = AntFinderPatternFinder::HandlePossibleCenter(image, stateCount, i, maxJ, possibleCenters,f);
            if (confirmed) {
                iSkip = stateCount[0];
                if (hasSkipped) {
                    // Found a third one
                    done = HaveMultiplyConfirmedCenters(possibleCenters);
                }
            }
        }
    }

    auto bestPatterns = SelectBestPatterns(possibleCenters);
    if (bestPatterns.empty())
        return {};

    OrderBestPatterns(bestPatterns);

    return { bestPatterns[0], bestPatterns[1], bestPatterns[2]};
}

static float CenterFromEnd(const StateCount& stateCount, int end)
{
    return (float)(end - stateCount[2]) - stateCount[1] / 2.0f;
}

static std::pair<float ,int> CrossCheckVertical(const BitMatrix& image, int startI, int centerJ, int maxCount,HookFunction f)
{
    StateCount stateCount = {};
    int maxI = image.height();
//    f(6,0,0,0);
    // Start counting up from center
    int i = startI;
    while (i >= 0 && !image.get(centerJ, i)) {
        stateCount[1]++;
        f(7,0,centerJ,i);
        i--;
    }
    if (i < 0) {
        return {std::numeric_limits<float>::quiet_NaN(),-1};
    }
    while (i >= 0 && image.get(centerJ, i) && stateCount[0] <= maxCount) {
        stateCount[0]++;
//        f(7,0,centerJ,i);
        i--;
    }
    // If already too many modules in this state or ran off the edge:
    if (i < 0 || stateCount[0] > maxCount) {
        return {std::numeric_limits<float>::quiet_NaN(),-1};
    }

    // Now also count down from center
    i = startI + 1;
    while (i < maxI && !image.get(centerJ, i)) {
        stateCount[1]++;
//        f(7,0,centerJ,i);
        i++;
    }
    if (i == maxI) {
        return {std::numeric_limits<float>::quiet_NaN(),-1};
    }
    while (i < maxI && image.get(centerJ, i) && stateCount[2] < maxCount) {
        stateCount[2]++;
//        f(7,0,centerJ,i);
        i++;
    }
    if (i == maxI || stateCount[2] >= maxCount) {
        return {std::numeric_limits<float>::quiet_NaN(),-1};
    }
//    f(8, reinterpret_cast<long>(&image),0,0);
    return {AntFinderPatternFinder::FoundPatternCross(stateCount) ? CenterFromEnd(stateCount, i) : std::numeric_limits<float>::quiet_NaN(),Accumulate(stateCount, 0)};
}

static bool FoundPatternDiagonal(const std::array<int,5>& stateCount) {
    int totalModuleSize = 0;
    for (int i = 0; i < 5; i++) {
        int count = stateCount[i];
        if (count == 0) {
            return false;
        }
        totalModuleSize += count;
    }
    if (totalModuleSize < 7) {
        return false;
    }
    float moduleSize = totalModuleSize / 7.0f;
    float maxVariance = moduleSize / 1.333f;
    // Allow less than 75% variance from 1-1-3-1-1 proportions
    return std::abs(moduleSize - stateCount[0]) < maxVariance &&
            std::abs(moduleSize - stateCount[1]) < maxVariance &&
            std::abs(3.0f * moduleSize - stateCount[2]) < 3 * maxVariance &&
            std::abs(moduleSize - stateCount[3]) < maxVariance &&
            std::abs(moduleSize - stateCount[4]) < maxVariance;
}

static bool CrossCheckDiagonal(const BitMatrix& image, int centerI, int centerJ)
{
    std::array<int, 5> stateCount = {};

    // Start counting up, left from center finding black center mass
    int i = 0;
    while (centerI >= i && centerJ >= i && image.get(centerJ - i, centerI - i)) {
        stateCount[2]++;
        i++;
    }
    if (stateCount[2] == 0) {
        return false;
    }

    // Continue up, left finding white space
    while (centerI >= i && centerJ >= i && !image.get(centerJ - i, centerI - i)) {
        stateCount[1]++;
        i++;
    }
    if (stateCount[1] == 0) {
        return false;
    }

    // Continue up, left finding black border
    while (centerI >= i && centerJ >= i && image.get(centerJ - i, centerI - i)) {
        stateCount[0]++;
        i++;
    }
    if (stateCount[0] == 0) {
        return false;
    }

    int maxI = image.height();
    int maxJ = image.width();

    // Now also count down, right from center
    i = 1;
    while (centerI + i < maxI && centerJ + i < maxJ && image.get(centerJ + i, centerI + i)) {
        stateCount[2]++;
        i++;
    }

    while (centerI + i < maxI && centerJ + i < maxJ && !image.get(centerJ + i, centerI + i)) {
        stateCount[3]++;
        i++;
    }
    if (stateCount[3] == 0) {
        return false;
    }

    while (centerI + i < maxI && centerJ + i < maxJ && image.get(centerJ + i, centerI + i)) {
        stateCount[4]++;
        i++;
    }
    if (stateCount[4] == 0) {
        return false;
    }

    return FoundPatternDiagonal(stateCount);
}

bool AntFinderPatternFinder::HandlePossibleCenter(const BitMatrix& image, const StateCount& stateCount, int i, int j, std::vector<FinderPattern>& possibleCenters,HookFunction f)
{
    int stateCountTotal = Accumulate(stateCount, 0);
    float centerJ = CenterFromEnd(stateCount, j);
    int startX = (int)(j - stateCount[2] - 0.9 * stateCount[1]);
    auto v1 = CrossCheckVertical(image, i,startX, stateCount[1],f);
    auto centerI = v1.first;
    if (std::isnan(centerI)){
        return false;
    }

    if (5 * std::abs(v1.second - stateCountTotal) >= 3 * std::max(stateCountTotal,v1.second)){
        return false;
    }

    startX = (int)(j - stateCount[2] - 0.1 * stateCount[1]);
    auto v2 = CrossCheckVertical(image, i,startX, stateCount[1],f);
    if (std::isnan(centerI)){
        return false;
    }
    float estimatedModuleSize = stateCountTotal / 7.0f;

    if (std::abs(centerI - v2.first) >= 0.1 * estimatedModuleSize){
        return false;
    }

    if (std::isnan(centerJ) || !CrossCheckDiagonal(image, (int)centerI, (int)centerJ))
        return false;

    auto center = ZXing::FindIf(possibleCenters, [=](const FinderPattern& center) {
        return center.aboutEquals(estimatedModuleSize, centerI, centerJ);
    });
    if (center != possibleCenters.end())
        *center = center->combineEstimate(centerI, centerJ, estimatedModuleSize);
    else
        possibleCenters.emplace_back(centerJ, centerI, estimatedModuleSize);

    return true;
}

bool AntFinderPatternFinder::FoundPatternCross(const StateCount& stateCount){
    int totalModuleSize = Accumulate(stateCount, 0);
    if (totalModuleSize < 7) {
        return false;
    }
    float moduleSize = totalModuleSize / 7.0f;
    float maxVariance = moduleSize / 2.0f;
    // Allow less than 50% variance from 1-1-3-1-1 proportions
    return  std::abs(moduleSize - stateCount[0]) < maxVariance &&
            std::abs(5.0f * moduleSize - stateCount[1]) < 5 * maxVariance &&
            std::abs(moduleSize - stateCount[2]) < maxVariance;
}

}
}