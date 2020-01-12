/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "BirdgeBinaryBitmap.h"
#include "LuminanceSource.h"
#include "ByteArray.h"
#include "BitArray.h"
#include "BitMatrix.h"
#include "DecodeStatus.h"
#include "ZXNumeric.h"

#include <cassert>
#include <array>
#include <mutex>

namespace ZXing {



struct BirdgeBinaryBitmap::DataCache
{
	std::once_flag once;
	std::shared_ptr<const BitMatrix> matrix;
};

BirdgeBinaryBitmap::BirdgeBinaryBitmap(const std::shared_ptr<const LuminanceSource>& source, bool pureBarcode) :
	GlobalHistogramBinarizer(source, pureBarcode),
	_cache(new DataCache)
{
}

BirdgeBinaryBitmap::~BirdgeBinaryBitmap()
{
}


/**
* Calculates the final BitMatrix once for all requests. This could be called once from the
* constructor instead, but there are some advantages to doing it lazily, such as making
* profiling easier, and not doing heavy lifting when callers don't expect it.
*/
static void InitBlackMatrix(const LuminanceSource& source, std::shared_ptr<const BitMatrix>& outMatrix)
{
	int width = source.width();
	int height = source.height();
	auto matrix = std::make_shared<BitMatrix>(width, height);
	ByteArray buffer;
	int stride;
	const uint8_t* luminances = source.getMatrix(buffer, stride);
	for (int y = 0; y < height; y++) {
		int offset = y * stride;
		for (int x = 0; x < width; x++) {
			int pixel = luminances[offset + x];
			if (pixel == 0){
				matrix->set(x, y);
			}
		}
	}
	outMatrix = matrix;
}

std::shared_ptr<const BitMatrix>
BirdgeBinaryBitmap::getBlackMatrix() const
{
	std::call_once(_cache->once, &InitBlackMatrix, std::cref(*_source), std::ref(_cache->matrix));
	return _cache->matrix;
}

std::shared_ptr<BinaryBitmap>
BirdgeBinaryBitmap::newInstance(const std::shared_ptr<const LuminanceSource>& source) const
{
	return std::make_shared<BirdgeBinaryBitmap>(source, _pureBarcode);
}

} // ZXing
