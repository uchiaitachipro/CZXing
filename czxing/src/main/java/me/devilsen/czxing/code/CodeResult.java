package me.devilsen.czxing.code;

import me.devilsen.czxing.thread.FrameData;

/**
 * desc: code result model
 * date: 2019/08/17
 *
 * @author : dongsen
 */
public class CodeResult {

    private BarcodeFormat format;
    private String text;
    private float[] points;
    private double cameraLight;
    private FrameData previewData;

    CodeResult(BarcodeFormat format, String text) {
        this.format = format;
        this.text = text;
    }

    public CodeResult(String content,double cameraLight, int formatIndex, float[] points,FrameData data) {
        this.text = content;
        this.cameraLight = cameraLight;
        if (formatIndex < 0) {
            this.format = BarcodeFormat.QR_CODE;
        } else {
            this.format = BarcodeFormat.values()[formatIndex];
        }
        this.points = points;
        this.previewData = data;
    }

    public void setPoint(float[] lists) {
        points = lists;
    }

    public BarcodeFormat getFormat() {
        return format;
    }

    public String getText() {
        return text;
    }

    public float[] getPoints() {
        return points;
    }

    public double getCameraLight() {
        return cameraLight;
    }

    public FrameData getPreviewData() {
        return previewData;
    }

    @Override
    public String toString() {
        return "text: " + text + " format: " + getFormat() + " points: " + getPointsString();
    }

    private String getPointsString() {
        StringBuilder stringBuilder = new StringBuilder();
        int i = 0;
        for (float list : points) {
            i++;
            stringBuilder.append(list).append("  ");
            if (i % 2 == 0) {
                stringBuilder.append("\n");
            }
        }
        return stringBuilder.toString();
    }
}
