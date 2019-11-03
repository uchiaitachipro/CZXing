package me.devilsen.czxing.code;

import android.graphics.Bitmap;
import android.util.Log;

import java.util.concurrent.atomic.AtomicInteger;

import me.devilsen.czxing.thread.Dispatcher;
import me.devilsen.czxing.thread.FrameData;
import me.devilsen.czxing.thread.ProcessRunnable;
import me.devilsen.czxing.util.BarCodeUtil;

public class BarcodeReader {

    private long _nativePtr;
    private static BarcodeReader instance;
    private int[] decodeStrategies = new int[]{
            NativeSdk.STRATEGY_THRESHOLD,
            NativeSdk.STRATEGY_ADAPTIVE_THRESHOLD_CLOSELY
    };
    private Dispatcher dispatcher;
    private AtomicInteger counter = new AtomicInteger(0);

    private boolean applyAllDecodeStrategies = false;

    public static BarcodeReader getInstance() {
        if (instance == null) {
            synchronized (BarcodeReader.class) {
                if (instance == null) {
                    instance = new BarcodeReader();
                }
            }
        }
        return instance;
    }

    private BarcodeReader() {
        setBarcodeFormat(BarcodeFormat.QR_CODE);
        dispatcher = new Dispatcher();
    }

    public void setBarcodeFormat(BarcodeFormat... formats) {
        int[] nativeFormats = new int[formats.length];
        for (int i = 0; i < formats.length; ++i) {
            nativeFormats[i] = formats[i].ordinal();
        }
        _nativePtr = NativeSdk.getInstance().createInstance(nativeFormats);
        setDecodeStrategies(decodeStrategies);
        setApplyAllDecodeStrategies(applyAllDecodeStrategies);
    }

    public void setDecodeStrategies(int[] strategies) {
        if (_nativePtr <= 0) {
            return;
        }
        this.decodeStrategies = strategies;
        NativeSdk.getInstance().setDecodeStrategies(_nativePtr, this.decodeStrategies);
    }

    public void setApplyAllDecodeStrategies(boolean applyAllDecodeStrategies) {
        if (_nativePtr <= 0){
            return;
        }
        this.applyAllDecodeStrategies = applyAllDecodeStrategies;
        NativeSdk.getInstance().applyAllDecodeStrategies(_nativePtr,applyAllDecodeStrategies);
    }

    public CodeResult read(Bitmap bitmap) {
        if (bitmap == null) {
            BarCodeUtil.e("bitmap is null");
            return null;
        }
        int imgWidth = bitmap.getWidth();
        int imgHeight = bitmap.getHeight();
        Object[] result = new Object[2];
        int resultFormat = NativeSdk.getInstance().readBarcode(_nativePtr, bitmap, 0, 0, imgWidth, imgHeight, result);
        if (resultFormat >= 0) {
            CodeResult decodeResult = new CodeResult(BarcodeFormat.values()[resultFormat], (String) result[0]);
            if (result[1] != null) {
                decodeResult.setPoint((float[]) result[1]);
            }
            return decodeResult;
        }
        return null;
    }

    public CodeResult read(byte[] data, int cropLeft, int cropTop, int cropWidth, int cropHeight, int rowWidth, int rowHeight) {
        try {
            int strategyIndex  = counter.getAndIncrement();

            if (data == null || data.length < 3){
                return null;
            }

            NativeSdk.getInstance().readBarcodeByte(_nativePtr, data, cropLeft, cropTop, cropWidth, cropHeight, rowWidth, rowHeight,strategyIndex);
            if (counter.get() >= 100000){
                counter.set(0);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    public CodeResult readAsync(byte[] data, int cropLeft, int cropTop, int cropWidth, int cropHeight, int rowWidth, int rowHeight) {
        FrameData frameData = new FrameData(data, cropLeft,
                cropTop, cropWidth,
                cropHeight, rowWidth, rowHeight);
        ProcessRunnable pr = dispatcher.newRunnable(frameData,null);
        pr.enqueue();
        return null;
    }

    public void prepareRead() {
        NativeSdk.getInstance().prepareRead(_nativePtr);
    }

    public void stopRead() {
        NativeSdk.getInstance().stopRead(_nativePtr);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            if (_nativePtr != 0) {
                NativeSdk.getInstance().destroyInstance(_nativePtr);
                _nativePtr = 0;
            }
        } finally {
            super.finalize();
        }
    }

    public Dispatcher getDispatcher(){
        return dispatcher;
    }

    public void setReadCodeListener(ReadCodeListener readCodeListener) {
        NativeSdk.getInstance().setReadCodeListener(readCodeListener);
    }

    public interface ReadCodeListener {
        void onReadCodeResult(CodeResult result);

        void onAnalysisBrightness(boolean isDark);
    }
}
