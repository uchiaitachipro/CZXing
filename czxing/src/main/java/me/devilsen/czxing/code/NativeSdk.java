package me.devilsen.czxing.code;

import android.graphics.Bitmap;

/**
 * desc: Jni connector
 * date: 2019/08/17 0017
 *
 * @author : dongsen
 */
public class NativeSdk {

    public static final int STRATEGY_RAW_PICTURE = 1;
    public static final int STRATEGY_THRESHOLD = 2;
    public static final int STRATEGY_ADAPTIVE_THRESHOLD_CLOSELY = 4;
    public static final int STRATEGY_COLOR_EXTRACT = 8;
    public static final int STRATEGY_ADAPTIVE_THRESHOLD_REMOTELY = 32;

    private NativeSdk() {
    }

    public static NativeSdk getInstance() {
        return Holder.instance;
    }

    private static class Holder {
        private static final NativeSdk instance = new NativeSdk();
    }

    private BarcodeReader.ReadCodeListener readCodeListener;

    void setReadCodeListener(BarcodeReader.ReadCodeListener readCodeListener) {
        this.readCodeListener = readCodeListener;
    }

    /**
     * Native Callback for scan result
     *
     * @param content     识别出的文字
     * @param formatIndex 格式
     * @param points      定位点的位置
     */
    public void onDecodeCallback(String content,double cameraLight, int formatIndex, float[] points) {
        if (readCodeListener != null) {
            readCodeListener.onReadCodeResult(new CodeResult(content, cameraLight,formatIndex, points));
        }
    }

    public Bitmap getQRCodeArea(Bitmap sourceBitmap){
        Bitmap result = Bitmap.createBitmap(sourceBitmap.getWidth(),
                sourceBitmap.getHeight(),
                Bitmap.Config.ARGB_8888);
        drawQRCodeArea(sourceBitmap,result);
        return result;
    }

    /**
     * Native callback for brightness
     *
     * @param isDark true : bright too low
     */
    public void onBrightnessCallback(boolean isDark) {
        if (readCodeListener != null) {
            readCodeListener.onAnalysisBrightness(isDark);
        }
    }

    // read
    native long createInstance(int[] formats);

    native void destroyInstance(long objPtr);

    native int readBarcode(long objPtr, Bitmap bitmap, int left, int top, int width, int height, Object[] result);

    native int readBarcodeByte(long objPtr, byte[] bytes, int left, int top, int width, int height, int rowWidth, int rowHeight,int strategyIndex);

    native void prepareRead(long objPtr);

    native void stopRead(long objPtr);

    // write
    native int writeCode(String content, int width, int height, int color, String format, Object[] result);

    native void drawQRCodeArea(Bitmap src,Bitmap dest);

    native void setDecodeStrategies(long objPtr,int[] strategyArray);

    static {
        System.loadLibrary("czxing");
    }

}
