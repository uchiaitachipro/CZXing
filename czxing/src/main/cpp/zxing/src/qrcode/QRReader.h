#pragma once
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

#include "Reader.h"

#include <sstream>
#include <string>

namespace ZXing {

class DecodeHints;

namespace QRCode {


/**
* This implementation can detect and decode QR Codes in an image.
*
* @author Sean Owen
*/
class Reader : public ZXing::Reader
{
public:

//	typedef void (*HookFunction)(int,long,long);

	explicit Reader(const DecodeHints& hints);
//	explicit Reader(const DecodeHints& hints,HookFunction function);
	Result decode(const BinaryBitmap& image) const override;

private:
	bool _tryHarder;
	std::string _charset;
//	HookFunction _hook = 0L;
//
//	void notifyHook(int phrase,long p1,long p2) const {
//		if (_hook == 0){
//			return;
//		}
//		_hook(phrase,p1,p2);
//	}
};

} // QRCode
} // ZXing
