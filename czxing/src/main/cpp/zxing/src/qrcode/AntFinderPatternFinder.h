//
// Created by uchiachen on 2020-01-22.
//

#ifndef CZXING_ANTFINDERPATTERNFINDER_H
#define CZXING_ANTFINDERPATTERNFINDER_H

#include <array>
#include <vector>
namespace ZXing {
class BitMatrix;

namespace QRCode 
{
class FinderPattern;
class FinderPatternInfo;

class AntFinderPatternFinder
{
public:
    using StateCount = std::array<int, 3>;

    static FinderPatternInfo Find(const BitMatrix& image, bool tryHarder);

    static bool FoundPatternCross(const StateCount& stateCount);

    static bool HandlePossibleCenter(const BitMatrix& image, const StateCount& stateCount, int i, int j, std::vector<FinderPattern>& possibleCenters);
};

}

}

#endif //CZXING_ANTFINDERPATTERNFINDER_H
