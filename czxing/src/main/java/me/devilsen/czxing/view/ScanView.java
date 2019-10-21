package me.devilsen.czxing.view;

import android.content.Context;
import android.graphics.Point;
import android.text.TextUtils;
import android.util.AttributeSet;

import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

import me.devilsen.czxing.ScanResult;
import me.devilsen.czxing.ScannerManager;
import me.devilsen.czxing.code.BarcodeFormat;
import me.devilsen.czxing.code.BarcodeReader;
import me.devilsen.czxing.code.CodeResult;
import me.devilsen.czxing.util.BarCodeUtil;

import static me.devilsen.czxing.ScannerManager.FIND_POTENTIAL_AREA_FOCUS;
import static me.devilsen.czxing.ScannerManager.FIND_POTENTIAL_AREA_ZOOM;

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
    private long nextStartTime;
    private BarcodeReader reader;
    private ScannerManager.ScanOption option;
    private long continuousScanStep = 0;
    private AtomicInteger failCount = new AtomicInteger(0);

    private boolean isFirstFrame = true;
    private long startTime = -1;
    private long scanSuccessDuration = -1;

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

        if (isFirstFrame) {
            startTime = System.currentTimeMillis();
            isFirstFrame = false;
        }

        reader.readAsync(data, left, top, width, height, rowWidth, rowHeight);
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
        nextStartTime = -1;
        isStop = true;
        isFirstFrame = true;
        startTime = -1;
        scanSuccessDuration = -1;
        reader.setReadCodeListener(null);
    }

    @Override
    public void onReadCodeResult(CodeResult result) {
        if (result == null) {
            return;
        }
        BarCodeUtil.d("result : " + result.toString());

        if (nextStartTime >= System.currentTimeMillis()) {
            return;
        }

        if (!TextUtils.isEmpty(result.getText()) && !isStop) {
            scanSuccessDuration = System.currentTimeMillis() - startTime;
            if (option == null || option.getContinuousScanTime() < 0) {
                isStop = true;
                reader.stopRead();
            } else {
//                resetZoom();
                nextStartTime = System.currentTimeMillis() + continuousScanStep;
                startTime = nextStartTime;

            }

            if (mScanListener != null) {
                mScanListener.onScanSuccess(new ScanResult(
                        result.getText(),
                        result.getFormat(),
                        result.getCameraLight(),
                        scanSuccessDuration,
                        getCurrentZoom(),
                        getCurrentExposureCompensation()));
                return;
            }
        } else if (result.getPoints() != null) {
//            mScanBoxView.currentResult = result;
            mScanBoxView.postInvalidate();
            if (option.getPotentialAreaStrategies() == FIND_POTENTIAL_AREA_ZOOM) {
                tryZoom(result);
            } else if (option.getPotentialAreaStrategies() == FIND_POTENTIAL_AREA_FOCUS) {
                tryFocus(result);
            }

        }

        // 失败超过5次重新聚焦
        if (failCount.get() >= 5) {
            Point p = mScanBoxView.getScanBoxCenter();
            mCameraSurface.handleFocus(p.x, p.y);
            failCount.set(0);
        } else {
            failCount.addAndGet(1);
        }
        if (mScanListener != null) {
            mScanListener.onScanFail();
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

    public void restartPreviewAfterDelay(long delayMS) {
        if (option == null) {
            return;
        }
        option.setContinuousScanTime(delayMS);
        if (isStop) {
            startScan();
        }
    }

    @Override
    public void onFlashLightClick() {
        mCameraSurface.toggleFlashLight(isDark);
    }

    public void applyScanOption(ScannerManager.ScanOption scanOption) {
        this.option = scanOption;
        setScanMode();
        this.mScanBoxView.applyScanOptions(option);
        if (option == null) {
            return;

        }
        if (option.getCaptureMode() != -1) {
            setPreviewMode(option.getCaptureMode());
        }
        if (option.getApplyFrameStrategies() != null) {
            List<Integer> list = option.getApplyFrameStrategies();
            int[] ret = new int[list.size()];
            for (int i = 0; i < ret.length; i++) {
                ret[i] = list.get(i).intValue();
            }
            reader.setDecodeStrategies(ret);
        }
    }

    public void onResume() {
        openCamera(); // 打开后置摄像头开始预览，但是并未开始识别
        startScan();  // 显示扫描框，并开始识别
    }

    public void onPause() {
        stopScan();
        closeCamera(); // 关闭摄像头预览，并且隐藏扫描框
    }

    private void setScanMode() {

        if (option.getScanMode() == -1) {
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
