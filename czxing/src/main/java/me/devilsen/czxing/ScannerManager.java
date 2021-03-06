package me.devilsen.czxing;

import android.content.Context;
import android.content.Intent;
import android.os.Parcel;
import android.os.Parcelable;

import java.util.ArrayList;
import java.util.List;

import me.devilsen.czxing.view.ScanActivityDelegate;

/**
 * desc :
 * date : 2019/07/31
 *
 * @author : dongsen
 */
public class ScannerManager {

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

    public ScannerManager setBorderColor(int borderColor) {
        scanOption.borderColor = borderColor;
        return this;
    }

    public ScannerManager setScanMode(int scanMode) {
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

    public ScannerManager continuousScan() {
        scanOption.continuousScan = true;
        return this;
    }

    public ScannerManager setScanLineColors(List<Integer> scanLineColors) {
        scanOption.scanLineColors = scanLineColors;
        return this;
    }

    public ScannerManager setOnScanResultDelegate(ScanActivityDelegate.OnScanDelegate delegate) {
        ScanActivityDelegate.getInstance().setScanResultDelegate(delegate);
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

        private int cornerColor;
        private int borderColor;
        private int scanMode;
        private String title;
        private boolean showAlbum = true;
        private boolean continuousScan;
        private List<Integer> scanLineColors;

        public int getCornerColor() {
            return cornerColor;
        }

        public int getBorderColor() {
            return borderColor;
        }

        public int getScanMode(){
            return scanMode;
        }

        public String getTitle() {
            return title;
        }

        public boolean isShowAlbum() {
            return showAlbum;
        }

        public boolean isContinuousScan(){
            return continuousScan;
        }

        public List<Integer> getScanLineColors() {
            return scanLineColors;
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeInt(this.cornerColor);
            dest.writeInt(this.borderColor);
            dest.writeInt(this.scanMode);
            dest.writeString(this.title);
            dest.writeByte(this.showAlbum ? (byte) 1 : (byte) 0);
            dest.writeByte(this.continuousScan ? (byte) 1 : (byte) 0);
            dest.writeList(this.scanLineColors);
        }

        public ScanOption() {
        }

        protected ScanOption(Parcel in) {
            this.cornerColor = in.readInt();
            this.borderColor = in.readInt();
            this.scanMode = in.readInt();
            this.title = in.readString();
            this.showAlbum = in.readByte() != 0;
            this.continuousScan = in.readByte() != 0;
            this.scanLineColors = new ArrayList<>();
            in.readList(this.scanLineColors, Integer.class.getClassLoader());
        }

        public static final Creator<ScanOption> CREATOR = new Creator<ScanOption>() {
            @Override
            public ScanOption createFromParcel(Parcel source) {
                return new ScanOption(source);
            }

            @Override
            public ScanOption[] newArray(int size) {
                return new ScanOption[size];
            }
        };
    }
}
