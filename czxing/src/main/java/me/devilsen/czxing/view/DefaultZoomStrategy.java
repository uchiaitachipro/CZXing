package me.devilsen.czxing.view;

import android.graphics.Rect;
import android.graphics.RectF;
import android.hardware.Camera;

import me.devilsen.czxing.code.CodeResult;
import me.devilsen.czxing.util.BarCodeUtil;

public class DefaultZoomStrategy implements IZoomQRCodeStrategy {

    private static final float DETECT_CODE_AREA_RATIO = 0.5f;
    private static final int MIN_VERSION_QRCODE = 21 * 21;

    private static final float FIRST_ZOOM_RATIO = 4f;
    private static final float SECOND_ZOOM_RATIO = 2f;
    private static final float THIRD_ZOOM_RATIO = 1f;

    private static final int MAX_FAIL_COUNT = 16;

    private static final int DEFAULT_ZOOM_SCALE = 4;


    private BarCoderView coderView;
    private int zoomCount = 0;
    private int failCount;

    public DefaultZoomStrategy(BarCoderView coderView) {
        this.coderView = coderView;
    }

    @Override
    public boolean needZoom(CodeResult codeResult) {
        if (coderView.getCamera() == null) {
            return false;
        }

        Rect boxRect = coderView.getScanBox().getScanBoxRect();
        if (boxRect == null || !checkRectValid(boxRect)) {
            return false;
        }

        if (codeResult.getPoints().length < 6) {
            return false;
        }
        float[] points = codeResult.getPoints();
        RectF detectArea = new RectF(points[0], points[1], points[2], points[5]);
        BarCodeUtil.d("detect area: " + detectArea + " width: " + detectArea.width() + " height: " + detectArea.height());

        float ratio = Math.min(detectArea.width(), detectArea.height())
                / Math.max(detectArea.width(), detectArea.height());

        if (ratio < DETECT_CODE_AREA_RATIO) {
            return false;
        }

        // 只针对二维码启用放大功能
        if (detectArea.width() * detectArea.height() < MIN_VERSION_QRCODE) {
            return false;
        }

        return true;
    }

    @Override
    public int calculateZoomRatio(CodeResult codeResult) {

        if (coderView.getCamera() == null) {
            return 0;
        }

        Rect boxRect = coderView.getScanBox().getScanBoxRect();
        int length = calculateDetectAreaLength(codeResult);


        if (length > boxRect.width() / DEFAULT_ZOOM_SCALE){
            return 0;
        }

        Camera.Parameters parameters = coderView.getCamera().getParameters();
        if (!parameters.isZoomSupported()) {
            return 0;
        }
        return calculateZoomValue(parameters);
    }

    private int calculateZoomValue(Camera.Parameters parameters){
        int zoomValue = parameters.getZoom();
        int maxZoom = parameters.getMaxZoom();
        double p = (int) Math.log10(maxZoom);
        float zoomStep = (int) (maxZoom / Math.pow(10, p) + .5f);
        if (zoomCount == 0){
            ++zoomCount;
            return (int) Math.floor(zoomStep * FIRST_ZOOM_RATIO);
        } else if (zoomCount == 1){
            ++zoomCount;
            int value = (int) Math.ceil(zoomStep * SECOND_ZOOM_RATIO);
            return value >= 1 ? value : 1;
        } else if (zoomCount == 2){
            ++zoomCount;
            int value = (int) Math.ceil(zoomStep * THIRD_ZOOM_RATIO);
            return value >= 1 ? value : 1;
        }

        if (failCount < MAX_FAIL_COUNT){
            return 0;
        }

        zoomCount = 0;
        clearFailCount();
        return (- zoomValue) / 2;
    }

    private int calculateDetectAreaLength(CodeResult result) {
        int l1;
        float[] points = result.getPoints();

        float point1X = points[0];
        float point1Y = points[1];
        float point2X = points[2];
        float point2Y = points[3];
        float x1 = Math.abs(point1X - point2X);
        float y1 = Math.abs(point1Y - point2Y);
        l1 = (int) Math.sqrt(x1 * x1 + y1 * y1);


        float point3X = points[4];
        float point3Y = points[5];
        float x2 = Math.abs(point2X - point3X);
        float y2 = Math.abs(point2Y - point3Y);
        int l2 = (int) Math.sqrt(x2 * x2 + y2 * y2);
        return Math.min(l1, l2);
    }

    @Override
    public void increaseFailCount() {
        if (zoomCount <= 0) {
            return;
        }
        failCount++;
    }

    @Override
    public void clearFailCount() {
        failCount = 0;
    }

    private boolean checkRectValid(Rect r) {
        if (r.left <= 0 || r.right <= 0 || r.top <= 0 || r.bottom <= 0) {
            return false;
        }
        return true;
    }
}
