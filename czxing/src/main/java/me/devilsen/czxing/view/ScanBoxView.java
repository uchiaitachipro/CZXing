package me.devilsen.czxing.view;

import android.animation.ValueAnimator;
import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.Shader;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.view.animation.LinearInterpolator;

import java.util.List;

import me.devilsen.czxing.R;
import me.devilsen.czxing.ScannerManager;
import me.devilsen.czxing.util.BarCodeUtil;
import me.devilsen.czxing.util.BitmapUtil;

/**
 * @author : dongSen
 * date : 2019-07-01 15:51
 * desc : 扫描框
 */
public class ScanBoxView extends View {

    private final int MAX_BOX_SIZE = BarCodeUtil.dp2px(getContext(), 300);
    private final int SCAN_LINE_HEIGHT = BarCodeUtil.dp2px(getContext(), 1.5f);

    private Paint mPaint;
    private Paint mTxtPaint;
    private Paint mScanLinePaint;
    private Rect mFramingRect;
    private Rect mTextRect;

    private int mMaskColor;
    private int mTextColor;
    private int mTextColorBig;

    private int mTopOffset;
    private int mBoxSize;
    private int mBoxSizeOffset;

    private int mBorderColor;
    private float mBorderSize;

    private int mCornerColor;
    private int mCornerLength;
    private int mCornerSize;
    private float mHalfCornerSize;
    private int mBoxLeft;
    private int mBoxTop;

    private LinearGradient mScanLineGradient;
    private float mScanLinePosition;
    private ValueAnimator mScanLineAnimator;
    private int mTextSize;
    private int mTextSizeBig;

    private ScanBoxClickListener mFlashLightListener;
    // 是否处于黑暗环境
    private boolean isDark;
    private boolean mDrawCardText;
    private boolean isLightOn;

    private int mScanLineColor1;
    private int mScanLineColor2;
    private int mScanLineColor3;

    private Bitmap mLightOn;
    private Bitmap mLightOff;
    private int mFlashLightLeft;
    private int mFlashLightTop;
    private int mFlashLightRight;
    private int mFlashLightBottom;

    private String mFlashLightOnText;
    private String mFlashLightOffText;
    private String mScanNoticeText;
    private ScannerManager.ScanOption option;
    private boolean useBoxSize = true;
    private int scanBoxWidth;
    private int scanBoxHeight;

    public ScanBoxView(Context context) {
        this(context, null);
    }

    public ScanBoxView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public ScanBoxView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        Context context = getContext();
        mPaint = new Paint();
        mPaint.setAntiAlias(true);
        mScanLinePaint = new Paint();
        mTxtPaint = new Paint();
        mTxtPaint.setAntiAlias(true);

        Resources resources = getResources();

        mMaskColor = resources.getColor(R.color.czxing_line_mask);
        mTextColor = resources.getColor(R.color.czxing_text_normal);
        mTextColorBig = resources.getColor(R.color.czxing_text_big);

        mScanLineColor1 = resources.getColor(R.color.czxing_scan_1);
        mScanLineColor2 = resources.getColor(R.color.czxing_scan_2);
        mScanLineColor3 = resources.getColor(R.color.czxing_scan_3);

        mBoxSize = BarCodeUtil.dp2px(context, 200);
        mTopOffset = -BarCodeUtil.dp2px(context, 10);
        mBoxSizeOffset = BarCodeUtil.dp2px(context, 40);

        mBorderColor = resources.getColor(R.color.czxing_line_border);
        mBorderSize = BarCodeUtil.dp2px(context, 1);

        mCornerColor = resources.getColor(R.color.czxing_line_corner);
        mCornerLength = BarCodeUtil.dp2px(context, 20);
        mCornerSize = BarCodeUtil.dp2px(context, 3);
        mHalfCornerSize = 1.0f * mCornerSize / 2;

        mTextSize = BarCodeUtil.sp2px(context, 14);
        mTextSizeBig = BarCodeUtil.sp2px(context, 17);
        mTxtPaint.setTextSize(mTextSize);
        mTxtPaint.setTextAlign(Paint.Align.CENTER);
        mTxtPaint.setColor(Color.GRAY);
        mTxtPaint.setStyle(Paint.Style.FILL);

        mFlashLightOnText = getResources().getText(R.string.czxing_click_open_flash_light).toString();
        mFlashLightOffText = getResources().getText(R.string.czxing_click_close_flash_light).toString();
        mScanNoticeText = getResources().getText(R.string.czxing_scan_notice).toString();
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        calFramingRect();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (mFramingRect == null) {
            return;
        }

        // 画遮罩层
        drawMask(canvas);

        // 画边框线
        drawBorderLine(canvas);

        // 画四个直角的线
        drawCornerLine(canvas);

        // 画扫描线
        drawScanLine(canvas);

        // 画提示文本
        drawTipText(canvas);

        // 移动扫描线的位置
        moveScanLine();
    }


    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (event.getActionMasked() == MotionEvent.ACTION_DOWN) {
            float x = event.getX();
            float y = event.getY();

            if (x > mFlashLightLeft && x < mFlashLightRight &&
                    y > mFlashLightTop && y < mFlashLightBottom) {
                // 在亮度不够的情况下，或者在打开闪光灯的情况下才可以点击
                if (mFlashLightListener != null && (isDark || isLightOn)) {
                    mFlashLightListener.onFlashLightClick();
                    isLightOn = !isLightOn;
                    invalidate();
                }
                return true;
            }
        }
        return super.onTouchEvent(event);
    }

    public void setScanBoxClickListener(ScanBoxClickListener lightListener) {
        mFlashLightListener = lightListener;
    }

    private void calFramingRect() {
        if (mFramingRect != null) {
            return;
        }

        int viewWidth = getWidth();
        int viewHeight = getHeight();

        if (useBoxSize) {
            int minSize = Math.min(viewHeight, viewWidth);
            mBoxSize = Math.min(minSize * 3 / 5, MAX_BOX_SIZE);
            mBoxLeft = (viewWidth - mBoxSize) / 2;
            mBoxTop = (viewHeight - mBoxSize) / 2 + mTopOffset;
            mFramingRect = new Rect(mBoxLeft, mBoxTop, mBoxLeft + mBoxSize, mBoxTop + mBoxSize);
        } else {
            mBoxLeft = (viewWidth - scanBoxWidth) / 2;
            mBoxTop = (viewHeight - scanBoxHeight) / 2;
            mFramingRect = new Rect(mBoxLeft, mBoxTop, mBoxLeft + scanBoxWidth, mBoxTop + scanBoxHeight);
        }
    }

    private void drawMask(Canvas canvas) {
        int width = canvas.getWidth();
        int height = canvas.getHeight();

        if (mMaskColor != Color.TRANSPARENT) {
            mPaint.setStyle(Paint.Style.FILL);
            mPaint.setColor(mMaskColor);
            canvas.drawRect(0, 0, width, mFramingRect.top, mPaint);
            canvas.drawRect(0, mFramingRect.top, mFramingRect.left, mFramingRect.bottom + 1, mPaint);
            canvas.drawRect(mFramingRect.right + 1, mFramingRect.top, width, mFramingRect.bottom + 1, mPaint);
            canvas.drawRect(0, mFramingRect.bottom + 1, width, height, mPaint);
        }
    }

    private void drawBorderLine(Canvas canvas) {
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setColor(mBorderColor);
        mPaint.setStrokeWidth(mBorderSize);
        canvas.drawRect(mFramingRect, mPaint);
    }

    /**
     * 画四个直角的线
     */
    private void drawCornerLine(Canvas canvas) {
        if (mHalfCornerSize <= 0) {
            return;
        }
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setColor(mCornerColor);
        mPaint.setStrokeWidth(mCornerSize);
        canvas.drawLine(mFramingRect.left - mHalfCornerSize, mFramingRect.top, mFramingRect.left - mHalfCornerSize + mCornerLength, mFramingRect.top,
                mPaint);
        canvas.drawLine(mFramingRect.left, mFramingRect.top - mHalfCornerSize, mFramingRect.left, mFramingRect.top - mHalfCornerSize + mCornerLength,
                mPaint);
        canvas.drawLine(mFramingRect.right + mHalfCornerSize, mFramingRect.top, mFramingRect.right + mHalfCornerSize - mCornerLength, mFramingRect.top,
                mPaint);
        canvas.drawLine(mFramingRect.right, mFramingRect.top - mHalfCornerSize, mFramingRect.right, mFramingRect.top - mHalfCornerSize + mCornerLength,
                mPaint);

        canvas.drawLine(mFramingRect.left - mHalfCornerSize, mFramingRect.bottom, mFramingRect.left - mHalfCornerSize + mCornerLength,
                mFramingRect.bottom, mPaint);
        canvas.drawLine(mFramingRect.left, mFramingRect.bottom + mHalfCornerSize, mFramingRect.left,
                mFramingRect.bottom + mHalfCornerSize - mCornerLength, mPaint);
        canvas.drawLine(mFramingRect.right + mHalfCornerSize, mFramingRect.bottom, mFramingRect.right + mHalfCornerSize - mCornerLength,
                mFramingRect.bottom, mPaint);
        canvas.drawLine(mFramingRect.right, mFramingRect.bottom + mHalfCornerSize, mFramingRect.right,
                mFramingRect.bottom + mHalfCornerSize - mCornerLength, mPaint);
    }

    /**
     * 画扫描线
     */
    private void drawScanLine(Canvas canvas) {
        int width = useBoxSize ? mBoxSize : scanBoxWidth;
        if (mScanLineGradient == null) {

            mScanLineGradient = new LinearGradient(mBoxLeft, mBoxTop, mBoxLeft + width, mBoxTop,
                    new int[]{mScanLineColor1, mScanLineColor2, mScanLineColor3, mScanLineColor2, mScanLineColor1},
                    null,
                    Shader.TileMode.CLAMP);
            mScanLinePaint.setShader(mScanLineGradient);
        }

        canvas.drawRect(mBoxLeft,
                mBoxTop + mScanLinePosition,
                mBoxLeft + width,
                mBoxTop + mScanLinePosition + SCAN_LINE_HEIGHT,
                mScanLinePaint);
    }

    private void drawTipText(Canvas canvas) {
        int boxWidth = useBoxSize ? mBoxSize : scanBoxWidth;
        mTxtPaint.setTextSize(mTextSize);
        mTxtPaint.setColor(mTextColor);
        if (isDark || isLightOn) {
            canvas.drawText(isLightOn ? mFlashLightOffText : mFlashLightOnText,
                    mFramingRect.left + (boxWidth >> 1),
                    mFramingRect.bottom - mTextSize,
                    mTxtPaint);

            drawFlashLight(canvas);
        }

        if (!TextUtils.isEmpty(mScanNoticeText)) {
            canvas.drawText(mScanNoticeText,
                    mFramingRect.left + (boxWidth >> 1),
                    mFramingRect.bottom + mTextSize * 2,
                    mTxtPaint);
        }
        // 隐藏 我的卡片 文字
        if (!mDrawCardText) {
            return;
        }

        mTxtPaint.setTextSize(mTextSizeBig);
        mTxtPaint.setColor(mTextColorBig);
        String clickText = "我的名片";
        canvas.drawText(clickText,
                mFramingRect.left + (boxWidth >> 1),
                mFramingRect.bottom + mTextSize * 6,
                mTxtPaint);

        if (mTextRect == null) {
            mTextRect = new Rect();
            mTxtPaint.getTextBounds(clickText, 0, clickText.length() - 1, mTextRect);
            int width = mTextRect.width();
            int height = mTextRect.height();
            mTextRect.left = mFramingRect.left + (boxWidth >> 1) - 10;
            mTextRect.right = mTextRect.left + width + 10;
            mTextRect.top = mFramingRect.bottom + mTextSize * 6 - 10;
            mTextRect.bottom = mTextRect.top + height + 10;
        }
    }

    /**
     * 画手电筒
     */
    private void drawFlashLight(Canvas canvas) {
        if (mLightOff == null) {
            int unLightResId = option != null && option.getLightOffResource() != -1
                    ? option.getLightOffResource()
                    : R.drawable.ic_highlight_black_close_24dp;
            mLightOff = BitmapUtil.getBitmap(getContext(), unLightResId);
        }
        if (mLightOn == null) {
            int lightResId = option != null && option.getLightOnResource() != -1
                    ? option.getLightOnResource()
                    : R.drawable.ic_highlight_black_open_24dp;
            mLightOn = BitmapUtil.getBitmap(getContext(), lightResId);
        }
        if (mFlashLightLeft == 0 && mLightOff != null) {
            mFlashLightLeft = mFramingRect.left + ((mFramingRect.width() - mLightOff.getWidth()) >> 1);
            mFlashLightTop = mFramingRect.bottom - (mTextSize << 2);
            mFlashLightRight = mFlashLightLeft + mLightOff.getWidth();
            mFlashLightBottom = mFlashLightTop + mLightOff.getHeight();
        }
        drawFlashLightBitmap(canvas);
    }

    private void drawFlashLightBitmap(Canvas canvas) {
        if (isLightOn) {
            if (mLightOn != null) {
                canvas.drawBitmap(mLightOn, mFlashLightLeft, mFlashLightTop, mPaint);
            }
        } else {
            if (mLightOff != null) {
                canvas.drawBitmap(mLightOff, mFlashLightLeft, mFlashLightTop, mPaint);
            }
        }
    }

    private void moveScanLine() {
        if (mScanLineAnimator != null && mScanLineAnimator.isRunning()) {
            return;
        }
        final int boxSize = useBoxSize ? mBoxSize : scanBoxHeight;
        mScanLineAnimator = ValueAnimator.ofFloat(0, boxSize - mBorderSize * 2);
        mScanLineAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
            @Override
            public void onAnimationUpdate(ValueAnimator animation) {
                mScanLinePosition = (float) animation.getAnimatedValue();
                // 这里如果用postInvalidate会导致所在Activity的onStop和onDestroy方法阻塞，感谢lhhseraph的反馈
                postInvalidateOnAnimation(mBoxLeft,
                        ((int) (mBoxTop + mScanLinePosition - 10)),
                        mBoxLeft + boxSize,
                        ((int) (mBoxTop + mScanLinePosition + SCAN_LINE_HEIGHT + 10)));
            }
        });
        mScanLineAnimator.setDuration(2500);
        mScanLineAnimator.setInterpolator(new LinearInterpolator());
        mScanLineAnimator.setRepeatCount(ValueAnimator.INFINITE);
        mScanLineAnimator.start();
    }

    public Rect getScanBoxRect() {
        return mFramingRect;
    }

    public int getScanBoxWidth() {
        return useBoxSize ? mBoxSize : scanBoxWidth;
    }

    public int getScanBoxHeight() {
        return useBoxSize ? mBoxSize : scanBoxHeight;
    }

    /**
     * 有的手机得到的数据会有所偏移（如：华为P20），这里放大了获取到的数据
     */
    public int[] getScanBoxSizeExpand() {
        return new int[]{
                getScanBoxWidth() + mBoxSizeOffset,
                getScanBoxHeight() + mBoxSizeOffset
        };
    }

    /**
     * 获取数据偏移量
     */
    public int getExpandTop() {
        return mBoxSizeOffset;
    }

    public Point getScanBoxCenter() {
        int w = useBoxSize ? mBoxSize : scanBoxWidth;
        int h = useBoxSize ? mBoxSize : scanBoxHeight;
        int centerX = mBoxLeft + (w >> 1);
        int centerY = mBoxTop + (h >> 1);
        return new Point(centerX, centerY);
    }

    public void setDark(boolean dark) {
        if (this.isDark != dark) {
            postInvalidate();
        }
        isDark = dark;
    }

    /**
     * 设定四个角的颜色
     */
    public void setCornerColor(int color) {
        if (color == 0) {
            return;
        }
        this.mCornerColor = color;
    }

    /**
     * 设定扫描框的边框颜色
     */
    public void setBorderColor(int color) {
        if (color == 0) {
            return;
        }
        this.mBorderColor = color;
    }

    /**
     * 设定扫描线的颜色
     *
     * @param colors 渐变颜色组合
     */
    public void setScanLineColor(List<Integer> colors) {
        if (colors == null || colors.size() < 3) {
            return;
        }

        mScanLineColor1 = colors.get(0);
        mScanLineColor2 = colors.get(1);
        mScanLineColor3 = colors.get(2);

        mScanLineGradient = null;
    }

    /**
     * 隐藏 我的卡片 功能
     */
    public void hideCardText() {
        this.mDrawCardText = false;
    }

    public void startAnim() {
        if (mScanLineAnimator != null && !mScanLineAnimator.isRunning()) {
            mScanLineAnimator.start();
        }
    }

    public void stopAnim() {
        if (mScanLineAnimator != null && mScanLineAnimator.isRunning()) {
            mScanLineAnimator.cancel();
        }
    }

    public void applyScanOptions(ScannerManager.ScanOption option) {
        this.option = option;
        if (this.option == null) {
            return;
        }

        if (this.option.getScanBoxFrameMaskColor() != -1) {
            this.mMaskColor = this.option.getScanBoxFrameMaskColor();
        }
        setScanLineColor(this.option.getScanLineColors());
        if (this.option.getScanBoxFrameTopMargin() != -1) {
            this.mTopOffset = this.option.getScanBoxFrameTopMargin();
        }
        if (this.option.getBorderColor() != -1) {
            this.mBorderColor = this.option.getBorderColor();
        }
        if (this.option.getBorderSize() != -1) {
            this.mBorderSize = this.option.getBorderSize();
        }
        if (this.option.getCornerColor() != -1) {
            this.mCornerColor = this.option.getCornerColor();
        }
        if (this.option.getCornerLength() != -1) {
            this.mCornerLength = this.option.getCornerLength();
        }
        if (this.option.getCornerThickness() != -1) {
            this.mCornerSize = this.option.getCornerThickness();
        }
        mScanNoticeText = this.option.getScanBoxTips();
        if (this.option.getScanBoxTipsTextSize() != -1) {
            mTxtPaint.setTextSize(BarCodeUtil.sp2px(getContext(), this.option.getScanBoxTipsTextSize()));
        }

        if (this.option.getScanBoxWidth() == -1 || this.option.getScanBoxHeight() == -1) {
            useBoxSize = true;
        } else {
            useBoxSize = false;
            scanBoxWidth = this.option.getScanBoxWidth();
            scanBoxHeight = this.option.getScanBoxHeight();
        }

        if (this.option.getScanBoxOffset() != -1){
            mBoxSizeOffset = this.option.getScanBoxOffset();
        }
//        calFramingRect();
    }

    public interface ScanBoxClickListener {
        void onFlashLightClick();
    }
}
