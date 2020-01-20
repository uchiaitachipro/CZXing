package me.devilsen.czxing.thread;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * desc : 每一帧的数据
 * date : 2019-07-01 20:17
 *
 * @author : dongSen
 */
public class FrameData implements Parcelable {

    public byte[] data;
    public int left;
    public int top;
    public int width;
    public int height;
    public int rowWidth;
    public int rowHeight;

    public FrameData(){
        data = null;
        left = top = width = height = rowWidth = rowHeight = -1;
    }

    public FrameData(byte[] data, int left, int top, int width, int height, int rowWidth, int rowHeight) {
        this.data = data;
        this.left = left;
        this.top = top;
        this.width = width;
        this.height = height;
        this.rowWidth = rowWidth;
        this.rowHeight = rowHeight;
    }

    protected FrameData(Parcel in) {
        data = in.createByteArray();
        left = in.readInt();
        top = in.readInt();
        width = in.readInt();
        height = in.readInt();
        rowWidth = in.readInt();
        rowHeight = in.readInt();
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeByteArray(data);
        dest.writeInt(left);
        dest.writeInt(top);
        dest.writeInt(width);
        dest.writeInt(height);
        dest.writeInt(rowWidth);
        dest.writeInt(rowHeight);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<FrameData> CREATOR = new Creator<FrameData>() {
        @Override
        public FrameData createFromParcel(Parcel in) {
            return new FrameData(in);
        }

        @Override
        public FrameData[] newArray(int size) {
            return new FrameData[size];
        }
    };

    public byte[] getData() {
        return data;
    }

    public void setData(byte[] data) {
        this.data = data;
    }
}
