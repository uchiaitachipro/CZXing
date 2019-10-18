package me.devilsen.czxing;

import android.content.Context;
import android.content.Intent;
import android.os.Parcel;
import android.os.Parcelable;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import me.devilsen.czxing.view.ScanActivityDelegate;

/**
 * desc :
 * date : 2019/07/31
 *
 * @author : dongsen
 */
public class ScannerManager {

    /**
     * Decode only UPC and EAN barcodes. This is the right choice for shopping apps which get
     * prices, reviews, etc. for products.
     */
    public static final int PRODUCT_MODE = 0;

    /**
     * Decode only 1D barcodes.
     */
    public static final int ONE_D_MODE = 1;

    /**
     * Decode only QR codes.
     */
    public static final int QR_CODE_MODE = 2;

    /**
     * Decode only Data Matrix codes.
     */
    public static final int DATA_MATRIX_MODE = 3;

    /**
     * support 1d 2d codes
     */
    public static final int ALL_MODE = 4;

    private Context context;
    private ScanOption scanOption;

    ScannerManager(Context context) {
        this.context = context;
        scanOption = new ScanOption();
    }

    public ScannerManager setCornerColor(int cornerColor) {
        scanOption.cornerColor = cornerColor;
        return this;
    }

    public ScannerManager setCornerThickness(int width){
        scanOption.cornerThickness = width;
        return this;
    }

    public ScannerManager setCornerLength(int length){
        scanOption.cornerLength = length;
        return this;
    }

    public ScannerManager setBorderColor(int borderColor) {
        scanOption.borderColor = borderColor;
        return this;
    }

    public ScannerManager setBorderSize(int borderWidth){
        this.scanOption.borderSize = borderWidth;
        return this;
    }

    public ScannerManager setCaptureMode(int captureMode) {
        scanOption.captureMode = captureMode;
        return this;
    }

    public ScannerManager setScanMode(int scanMode){
        scanOption.scanMode = scanMode;
        return this;
    }

    public ScannerManager setTitle(String title) {
        scanOption.title = title;
        return this;
    }

    public ScannerManager showAlbum(boolean showAlbum) {
        scanOption.showAlbum = showAlbum;
        return this;
    }

    public ScannerManager setcontinuousScanTime(long time) {
        scanOption.continuousScanTime = time;
        return this;
    }

    public ScannerManager setFrameCornerColor(int scanColor){
        scanOption.borderColor = scanColor;
        return this;
    }

    public ScannerManager setLaserLineColor(int color){
        scanOption.scanLineColors = (ArrayList<Integer>) Arrays.asList(color,color,color);
        return this;
    }

    public ScannerManager setScanLineColors(List<Integer> scanLineColors) {
        scanOption.scanLineColors = new ArrayList<>(scanLineColors);
        return this;
    }

    public ScannerManager setOnScanResultDelegate(ScanActivityDelegate.OnScanDelegate delegate) {
        ScanActivityDelegate.getInstance().setScanResultDelegate(delegate);
        return this;
    }

    public ScannerManager setFrameSize(int width,int height){
        scanOption.scanBoxWidth = width;
        scanOption.scanBoxHeight = height;
        return this;
    }

    public ScannerManager setTipText(String tips){
        scanOption.scanBoxTips = tips;
        return this;
    }

    public ScannerManager setFrameTopMargin(int topMargin){
        scanOption.scanBoxFrameTopMargin  =topMargin;
        return this;
    }

    public ScannerManager setFrameLeftMargin(int leftMargin){
        scanOption.scanBoxFrameLeftMargin = leftMargin;
        return this;
    }

    public ScannerManager setLightOffResourceId(int id){
        scanOption.lightOffResource = id;
        return this;
    }

    public ScannerManager setLightOnResourceId(int id){
        scanOption.lightOnResource = id;
        return this;
    }

    public ScannerManager setFrameStrategies(Integer... s){

        if (s == null || s.length < 0){
            return this;
        }

        ArrayList<Integer> list = new ArrayList<>(s.length);
        for (Integer i : s){
            list.add(i);
        }
        scanOption.applyFrameStrategies = list;
        return this;
    }

    public void start() {
        Intent intent = new Intent(context, ScanActivity.class);
        intent.putExtra("option", scanOption);
        context.startActivity(intent);
    }

    public ScannerManager setOnClickAlbumDelegate(ScanActivityDelegate.OnClickAlbumDelegate onClickAlbumDelegate) {
        ScanActivityDelegate.getInstance().setOnClickAlbumDelegate(onClickAlbumDelegate);
        return this;
    }

    public static class ScanOption implements Parcelable {

        private int cornerColor = -1;
        private int cornerLength = -1;
        private int cornerThickness = -1;
        private int borderColor = -1;
        private int borderSize = -1;
        private int captureMode = -1;
        private int scanMode = -1;
        private int scanBoxWidth = -1;
        private int scanBoxHeight = -1;
        private String scanBoxTips;
        private int scanBoxTipsTextSize = -1;
        private int scanBoxFrameTopMargin = -1;
        private int scanBoxFrameLeftMargin = -1;
        private int scanBoxFrameMaskColor = -1;
        private int lightOffResource = -1;
        private int lightOnResource = -1;
        private ArrayList<Integer> applyFrameStrategies;
        private String title;
        private boolean showAlbum = true;
        private long continuousScanTime = -1;
        private ArrayList<Integer> scanLineColors;

        public ScanOption(){}


        protected ScanOption(Parcel in) {
            cornerColor = in.readInt();
            cornerLength = in.readInt();
            cornerThickness = in.readInt();
            borderColor = in.readInt();
            borderSize = in.readInt();
            captureMode = in.readInt();
            scanMode = in.readInt();
            scanBoxWidth = in.readInt();
            scanBoxHeight = in.readInt();
            scanBoxTips = in.readString();
            scanBoxTipsTextSize = in.readInt();
            scanBoxFrameTopMargin = in.readInt();
            scanBoxFrameLeftMargin = in.readInt();
            scanBoxFrameMaskColor = in.readInt();
            lightOffResource = in.readInt();
            lightOnResource = in.readInt();
            applyFrameStrategies = (ArrayList<Integer>) in.readSerializable();
            title = in.readString();
            showAlbum = in.readByte() != 0;
            continuousScanTime = in.readLong();
            scanLineColors = (ArrayList<Integer>) in.readSerializable();
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeInt(cornerColor);
            dest.writeInt(cornerLength);
            dest.writeInt(cornerThickness);
            dest.writeInt(borderColor);
            dest.writeInt(borderSize);
            dest.writeInt(captureMode);
            dest.writeInt(scanMode);
            dest.writeInt(scanBoxWidth);
            dest.writeInt(scanBoxHeight);
            dest.writeString(scanBoxTips);
            dest.writeInt(scanBoxTipsTextSize);
            dest.writeInt(scanBoxFrameTopMargin);
            dest.writeInt(scanBoxFrameLeftMargin);
            dest.writeInt(scanBoxFrameMaskColor);
            dest.writeInt(lightOffResource);
            dest.writeInt(lightOnResource);
            dest.writeSerializable(applyFrameStrategies);
            dest.writeString(title);
            dest.writeByte((byte) (showAlbum ? 1 : 0));
            dest.writeLong(continuousScanTime);
            dest.writeSerializable(scanLineColors);
        }

        @Override
        public int describeContents() {
            return 0;
        }

        public static final Creator<ScanOption> CREATOR = new Creator<ScanOption>() {
            @Override
            public ScanOption createFromParcel(Parcel in) {
                return new ScanOption(in);
            }

            @Override
            public ScanOption[] newArray(int size) {
                return new ScanOption[size];
            }
        };

        public int getCornerColor() {
            return cornerColor;
        }

        public int getCornerLength() {
            return cornerLength;
        }

        public int getCornerThickness() {
            return cornerThickness;
        }


        public int getBorderColor() {
            return borderColor;
        }

        public int getBorderSize() {
            return borderSize;
        }

        public int getCaptureMode() {
            return captureMode;
        }

        public String getTitle() {
            return title;
        }

        public boolean isShowAlbum() {
            return showAlbum;
        }

        public long getContinuousScanTime() {
            return continuousScanTime;
        }

        public void setContinuousScanTime(long time){
            continuousScanTime = time;
        }

        public List<Integer> getScanLineColors() {
            return scanLineColors;
        }

        public int getScanBoxWidth() {
            return scanBoxWidth;
        }

        public int getScanBoxHeight() {
            return scanBoxHeight;
        }

        public String getScanBoxTips() {
            return scanBoxTips;
        }

        public int getScanBoxTipsTextSize() {
            return scanBoxTipsTextSize;
        }

        public int getScanBoxFrameTopMargin() {
            return scanBoxFrameTopMargin;
        }

        public int getScanBoxFrameLeftMargin() {
            return scanBoxFrameLeftMargin;
        }

        public int getScanBoxFrameMaskColor() {
            return scanBoxFrameMaskColor;
        }

        public int getScanMode() {
            return scanMode;
        }

        public int getLightOffResource() {
            return lightOffResource;
        }

        public int getLightOnResource() {
            return lightOnResource;
        }

        public ArrayList<Integer> getApplyFrameStrategies() {
            return applyFrameStrategies;
        }

    }
}
