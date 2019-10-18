package me.devilsen.czxing.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.text.TextUtils;
import android.util.AttributeSet;

import me.devilsen.czxing.ScannerManager;
import me.devilsen.czxing.code.BarcodeFormat;
import me.devilsen.czxing.code.BarcodeReader;
import me.devilsen.czxing.code.CodeResult;
import me.devilsen.czxing.util.BarCodeUtil;
import me.devilsen.czxing.util.SaveImageUtil;

/**
 * @author : dongSen
 * date : 2019-06-29 16:18
 * desc : 二维码界面使用类
 */
public class ScanView extends BarCoderView implements ScanBoxView.ScanBoxClickListener,
        BarcodeReader.ReadCodeListener {

    /**
     * 混合扫描模式（默认），扫描4次扫码框里的内容，扫描1次以屏幕宽为边长的内容
     */
    public static final int CAPTURE_MODE_MIX = 0;
    /**
     * 只扫描扫码框里的内容
     */
    public static final int CAPTURE_MODE_TINY = 1;
    /**
     * 扫描以屏幕宽为边长的内容
     */
    public static final int CAPTURE_MODE_BIG = 2;

    private static final int DARK_LIST_SIZE = 4;

    private boolean isStop;
    private boolean isDark;
    private int showCounter;
    private BarcodeReader reader;
    private ScannerManager.ScanOption option;

    public ScanView(Context context) {
        this(context, null);
    }

    public ScanView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public ScanView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        mScanBoxView.setScanBoxClickListener(this);
        reader = BarcodeReader.getInstance();
        reader.setBarcodeFormat(
                BarcodeFormat.QR_CODE,
                BarcodeFormat.CODABAR,
                BarcodeFormat.CODE_128,
                BarcodeFormat.EAN_13,
                BarcodeFormat.UPC_A
        );
    }

    @Override
    public void onPreviewFrame(byte[] data, int left, int top, int width, int height, int rowWidth, int rowHeight) {
        if (isStop) {
            return;
        }

        reader.read(data, left, top, width, height, rowWidth, rowHeight);
//        SaveImageUtil.saveData(data, left, top, width, height, rowWidth);
    }

    @Override
    public void startScan() {
        reader.setReadCodeListener(this);
        super.startScan();
        reader.prepareRead();
        isStop = false;
    }

    @Override
    public void stopScan() {
        super.stopScan();
        reader.stopRead();
        isStop = true;
        reader.setReadCodeListener(null);
    }

    @Override
    public void onReadCodeResult(CodeResult result) {
        if (result == null) {
            return;
        }
        BarCodeUtil.d("result : " + result.toString());

        if (!TextUtils.isEmpty(result.getText()) && !isStop) {
            isStop = true;
            reader.stopRead();
            if (mScanListener != null) {
                mScanListener.onScanSuccess(result.getText(), result.getFormat());
            }
        } else if (result.getPoints() != null) {
//            mScanBoxView.currentResult = result;
            mScanBoxView.postInvalidate();
            tryZoom(result);
        }
    }


    @Override
    public void onAnalysisBrightness(boolean isDark) {
        BarCodeUtil.d("isDark : " + isDark);

        if (isDark) {
            showCounter++;
            showCounter = showCounter > DARK_LIST_SIZE ? DARK_LIST_SIZE : showCounter;
        } else {
            showCounter--;
            showCounter = showCounter < 0 ? 0 : showCounter;
        }

        if (this.isDark) {
            if (showCounter <= 2) {
                this.isDark = false;
                mScanBoxView.setDark(false);
            }
        } else {
            if (showCounter >= DARK_LIST_SIZE) {
                this.isDark = true;
                mScanBoxView.setDark(true);
            }
        }
    }

    public void resetZoom() {
        setZoomValue(0);
    }

    @Override
    public void onFlashLightClick() {
        mCameraSurface.toggleFlashLight(isDark);
    }

    public void applyScanOption(ScannerManager.ScanOption scanOption) {
        this.option = scanOption;
        setScanMode();
        if (option != null && option.getCaptureMode() != -1){
            setPreviewMode(option.getCaptureMode());
        }
        this.mScanBoxView.applyScanOptions(option);
    }

    private void setScanMode() {

        if (option.getScanMode() == -1){
            return;
        }

        switch (option.getScanMode()) {
            case ScannerManager.PRODUCT_MODE:
            case ScannerManager.ONE_D_MODE:
                reader.setBarcodeFormat(BarcodeFormat.CODABAR,
                        BarcodeFormat.CODE_128,
                        BarcodeFormat.EAN_13,
                        BarcodeFormat.UPC_A);
                break;
            case ScannerManager.QR_CODE_MODE:
                reader.setBarcodeFormat(BarcodeFormat.QR_CODE);
                break;
            case ScannerManager.DATA_MATRIX_MODE:
                reader.setBarcodeFormat(BarcodeFormat.DATA_MATRIX);
                break;
            case ScannerManager.ALL_MODE:
            default:
                reader.setBarcodeFormat(
                        BarcodeFormat.QR_CODE,
                        BarcodeFormat.CODABAR,
                        BarcodeFormat.CODE_128,
                        BarcodeFormat.EAN_13,
                        BarcodeFormat.UPC_A);
        }
    }

}
