//
// Created by uchia on 2019-12-08.
//

#ifndef CZXING_ZXINGHOOKER_H
#define CZXING_ZXINGHOOKER_H
#include <src/qrcode/QRVersion.h>
class ZXingHooker {
public:

    enum HookPhrase{
        HOOK_THRESHOLD = 0,
        HOOK_PERSPECTIVE_TRANSFORM = 1,
        HOOK_BEFORE_GIRD_SAMPLING = 2,
        HOOK_AFTER_GIRD_SAMPLING = 3,
        HOOK_CALCULATE_VERSION = 4,
        HOOK_MODULE_SZIE = 5,
    };

    void hookHandler(int phrase,long  params1,long params2,long params3) const ;

private:

    void handleThreshold(long matrixPtr,long p2) const;
    void handleSaveModuleSize(long valuePtr) const;
    void handleCalculateVersion(long versionPtr) const;
    void handleFindPositionPattern(long matrixPtr,long finderPatternInfoPtr,long alignPatternPtr) const;

    void handleGirdSampling(long binaryImagePtr, bool before) const;

};


#endif //CZXING_ZXINGHOOKER_H
