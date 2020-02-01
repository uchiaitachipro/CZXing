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
        HOOK_FIND_POSITION_PATTERN = 1,
        HOOK_BEFORE_GIRD_SAMPLING = 2,
        HOOK_AFTER_GIRD_SAMPLING = 3,
        HOOK_CALCULATE_VERSION = 4,
        HOOK_MODULE_SIZE = 5,
        HOOK_START_CONFIRM_POSITION = 6,
        HOOK_CONFIRMING_POSITION = 7,
        HOOK_FINISH_CONFIRM_POSITION = 8
    };

    void HookHandler(int phrase, long  params1, long params2, long params3) const ;

private:

    void HandleThreshold(long matrixPtr, long p2) const;
    void HandleSaveModuleSize(long valuePtr) const;
    void HandleCalculateVersion(long versionPtr) const;
    void HandleFindPositionPattern(long matrixPtr, long finderPatternInfoPtr, long alignPatternPtr) const;
    void HandleConfirmPositionPattern(int phrase,long matrixPtr,long x,long y) const;
    void HandleGirdSampling(long binaryImagePtr, bool before) const;

};


#endif //CZXING_ZXINGHOOKER_H
