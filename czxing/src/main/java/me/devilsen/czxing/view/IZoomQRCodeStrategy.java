package me.devilsen.czxing.view;

import me.devilsen.czxing.code.CodeResult;

public interface IZoomQRCodeStrategy {
    boolean needZoom(CodeResult codeResult);
    int calculateZoomRatio(CodeResult codeResult);
    void increaseFailCount();
    void clearFailCount();
}
