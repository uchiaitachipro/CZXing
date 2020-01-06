//
// Created by uchia on 2019-12-18.
//
#pragma once


#include "ResultPoint.h"

namespace ZXing {
class BitMatrix;
namespace QRCode{
class AlignmentPattern;

class StrictAlignmentPatternFinder {


public:



    /**
    * <p>This method attempts to find the bottom-right alignment pattern in the image. It is a bit messy since
    * it's pretty performance-critical and so is written to be fast foremost.</p>
    * @param image image to search
    * @param startX left column from which to start searching
    * @param startY top row from which to start searching
    * @param width width of region to search
    * @param height height of region to search
    * @param moduleSize estimated module size so far
    *
    * @return {@link AlignmentPattern} if found
    * @throws NotFoundException if not found
    */
    static AlignmentPattern Find(const BitMatrix& image, int startX, int startY, int width, int height, float moduleSize);
};
}
}




