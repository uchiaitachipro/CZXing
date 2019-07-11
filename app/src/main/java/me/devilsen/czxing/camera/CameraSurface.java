package me.devilsen.czxing.camera;

import android.content.Context;
import android.graphics.Point;
import android.hardware.Camera;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.SurfaceView;

import me.devilsen.czxing.BarCodeUtil;
import me.devilsen.czxing.view.ScanListener;

/**
 * @author : dongSen
 * date : 2019-06-29 13:54
 * desc : 摄像头预览画面
 */
public class CameraSurface extends SurfaceView implements ICamera, SensorController.CameraFocusListener {

    // TODO camera2
    private CameraHelper mHelper;
    private SensorController mSensorController;

    private float mOldDist = 1f;
    private boolean mIsTouchFocusing;
    private Point focusCenter;
    private long mLastTouchTime;
    private boolean mFlashLightIsOpen;

    public CameraSurface(Context context) {
        this(context, null);
    }

    public CameraSurface(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CameraSurface(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        mHelper = new CameraHelper(getContext());

        mSensorController = new SensorController(context);
        mSensorController.setCameraFocusListener(this);
    }

    public void setCamera(Camera camera) {
        mHelper.setCamera(camera, this);
        if (camera == null) {
            return;
        }
        if (mHelper.isPreviewing()) {
            requestLayout();
        } else {
            startCameraPreview();
        }
    }

    @Override
    public void startCameraPreview() {
        mHelper.startCameraPreview();
        mSensorController.onStart();
    }

    @Override
    public void stopCameraPreview() {
        mHelper.stopCameraPreview();
        mSensorController.onStop();
    }


    @Override
    public void openFlashlight() {
        mHelper.openFlashlight();
    }

    @Override
    public void closeFlashlight() {
        mHelper.closeFlashlight();
    }

    public void toggleFlashLight(boolean isDark) {
        if (mFlashLightIsOpen) {
            closeFlashlight();
            mFlashLightIsOpen = false;
        } else if (isDark) {
            openFlashlight();
            mFlashLightIsOpen = true;
        }
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int width = getDefaultSize(getSuggestedMinimumWidth(), widthMeasureSpec);
        int height = getDefaultSize(getSuggestedMinimumHeight(), heightMeasureSpec);
        if (mHelper != null && mHelper.getCameraResolution() != null) {
            Point cameraResolution = mHelper.getCameraResolution();
            // 取出来的cameraResolution高宽值与屏幕的高宽顺序是相反的
            int cameraPreviewWidth = cameraResolution.x;
            int cameraPreviewHeight = cameraResolution.y;
            if (width * 1f / height < cameraPreviewWidth * 1f / cameraPreviewHeight) {
                float ratio = cameraPreviewHeight * 1f / cameraPreviewWidth;
                width = (int) (height / ratio + 0.5f);
            } else {
                float ratio = cameraPreviewWidth * 1f / cameraPreviewHeight;
                height = (int) (width / ratio + 0.5f);
            }
        }
        super.onMeasure(MeasureSpec.makeMeasureSpec(width, MeasureSpec.EXACTLY), MeasureSpec.makeMeasureSpec(height, MeasureSpec.EXACTLY));
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (mHelper == null || !mHelper.isPreviewing()) {
            return super.onTouchEvent(event);
        }

        if (event.getPointerCount() == 1) {
            int action = event.getAction() & MotionEvent.ACTION_MASK;
            if (action == MotionEvent.ACTION_DOWN) {
                long now = System.currentTimeMillis();
                if (now - mLastTouchTime < 300) {
                    doubleTap();
                    mLastTouchTime = 0;
                    return true;
                }
                mLastTouchTime = now;
            } else if (action == MotionEvent.ACTION_UP) {
                if (mIsTouchFocusing) {
                    return true;
                }
                mIsTouchFocusing = true;
                handleFocus(event.getX(), event.getY());
                BarCodeUtil.d("手指触摸，触发对焦测光");

                mIsTouchFocusing = false;
            }
        } else if (event.getPointerCount() == 2) {
            handleZoom(event);
        }
        return true;
    }

    /**
     * 双击放大
     */
    private void doubleTap() {
        mHelper.handleZoom(true, 5);
    }

    /**
     * 不动时对焦
     */
    @Override
    public void onFrozen() {
        BarCodeUtil.d("camera is frozen, start focus");
        handleFocus(focusCenter.x, focusCenter.y);
    }

    private void handleFocus(float x, float y) {
        if (!isPreviewing()) {
            return;
        }

        float centerX = x;
        float centerY = y;
        if (CameraUtil.isPortrait(getContext())) {
            float temp = centerX;
            centerX = centerY;
            centerY = temp;
        }
        int focusSize = CameraUtil.dp2px(getContext(), 100);
        mHelper.handleFocusMetering(centerX, centerY, focusSize, focusSize);
    }

    private void handleZoom(MotionEvent event) {
        switch (event.getAction() & MotionEvent.ACTION_MASK) {
            case MotionEvent.ACTION_POINTER_DOWN:
                mOldDist = CameraUtil.calculateFingerSpacing(event);
                break;
            case MotionEvent.ACTION_MOVE:
                float newDist = CameraUtil.calculateFingerSpacing(event);
                if (newDist > mOldDist) {
                    mHelper.handleZoom(true);
                } else if (newDist < mOldDist) {
                    mHelper.handleZoom(false);
                }
                break;
        }
    }

    public boolean isPreviewing() {
        return mHelper.isPreviewing();
    }

    public void setScanListener(ScanListener listener) {
        mHelper.setScanListener(listener);
    }

    public void setScanBoxPoint(Point scanBoxCenter) {
        if (focusCenter == null) {
            focusCenter = scanBoxCenter;
        }
    }
}