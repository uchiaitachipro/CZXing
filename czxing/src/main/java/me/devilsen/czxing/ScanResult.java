package me.devilsen.czxing;

import android.os.Parcel;
import android.os.Parcelable;

import me.devilsen.czxing.code.BarcodeFormat;
import me.devilsen.czxing.thread.FrameData;

/**
 * Created by uchia on 2019-10-21
 **/
public class ScanResult implements Parcelable {

    private String content;
    private BarcodeFormat format;
    private double cameraLight;
    private long scanSuccessDuration;
    private int zoomTimes;
    private int exposureCompensation;
    private FrameData previewData;

    public ScanResult(
            String content,
            BarcodeFormat format,
            double cameraLight,
            long scanSuccessDuration,
            int zoomTimes,
            int exposureCompensation,
            FrameData previewData){
        this.content = content;
        this.format = format;
        this.cameraLight = cameraLight;
        this.scanSuccessDuration = scanSuccessDuration;
        this.zoomTimes = zoomTimes;
        this.exposureCompensation = exposureCompensation;
        this.previewData = previewData;
    }

    protected ScanResult(Parcel in) {
        content = in.readString();
        format = BarcodeFormat.values()[in.readInt()];
        cameraLight = in.readDouble();
        scanSuccessDuration = in.readLong();
        zoomTimes = in.readInt();
        exposureCompensation = in.readInt();
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(content);
        dest.writeInt(format.ordinal());
        dest.writeDouble(cameraLight);
        dest.writeLong(scanSuccessDuration);
        dest.writeInt(zoomTimes);
        dest.writeInt(exposureCompensation);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<ScanResult> CREATOR = new Creator<ScanResult>() {
        @Override
        public ScanResult createFromParcel(Parcel in) {
            return new ScanResult(in);
        }

        @Override
        public ScanResult[] newArray(int size) {
            return new ScanResult[size];
        }
    };

    public String getContent() {
        return content;
    }

    public BarcodeFormat getFormat() {
        return format;
    }

    public double getCameraLight() {
        return cameraLight;
    }

    public long getScanSuccessDuration() {
        return scanSuccessDuration;
    }

    public int getZoomTimes() {
        return zoomTimes;
    }

    public int getExposureCompensation() {
        return exposureCompensation;
    }

    public FrameData getPreviewData() {
        return previewData;
    }


    @Override
    public String toString() {
        return "ScanResult{" +
                "content='" + content + '\'' +
                ", format=" + format +
                ", cameraLight=" + cameraLight +
                ", scanSuccessDuration=" + scanSuccessDuration +
                ", zoomTimes=" + zoomTimes +
                ", exposureCompensation=" + exposureCompensation +
                '}';
    }
}
