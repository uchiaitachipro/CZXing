package me.sam.czxing;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.provider.MediaStore;
import android.util.Log;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.yanzhenjie.permission.Action;
import com.yanzhenjie.permission.AndPermission;
import com.yanzhenjie.permission.runtime.Permission;
import com.zhihu.matisse.Matisse;
import com.zhihu.matisse.MimeType;

import java.util.Arrays;
import java.util.List;

import me.devilsen.czxing.ScanResult;
import me.devilsen.czxing.Scanner;
import me.devilsen.czxing.ScannerManager;
import me.devilsen.czxing.code.BarcodeReader;
import me.devilsen.czxing.code.CodeResult;
import me.devilsen.czxing.code.NativeSdk;
import me.devilsen.czxing.util.BarCodeUtil;
import me.devilsen.czxing.view.ScanActivityDelegate;
import me.devilsen.czxing.view.ScanView;
import me.sam.czxing.loaders.GlideEngine;

public class MainActivity extends AppCompatActivity {

    private static final int CODE_SELECT_IMAGE = 1;
    private static final int REQUEST_CODE_CHOOSE = 0x2019;
    private TextView resultTxt;
    private BarcodeReader reader;
    private HandlerThread thread = new HandlerThread("decode-");
    private Handler handler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        resultTxt = findViewById(R.id.text_view_result);
        reader = BarcodeReader.getInstance();
        requestPermission();
        thread.start();
        handler = new Handler(thread.getLooper());
    }

    public void scan(View view) {
        Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.test_boder_complex_7);
        CodeResult result = reader.read(bitmap);

        if (result == null) {
            Log.e("Scan >>> ", "no code");
            return;
        } else {
            Log.e("Scan >>> ", result.getText());
        }

        resultTxt.setText(result.getText());
    }

    public void write(View view) {
        Intent intent = new Intent(this, WriteCodeActivity.class);
        startActivity(intent);
    }

    public void writeQrCode(View view) {
        Intent intent = new Intent(this, WriteQRCodeActivity.class);
        startActivity(intent);
    }

    public void openScan(View view) {
//        Intent intent = new Intent(this, ScanActivity.class);
//        startActivity(intent);
        final Resources resources = getResources();
        List<Integer> scanColors = Arrays.asList(resources.getColor(R.color.scan_side), resources.getColor(R.color.scan_partial), resources.getColor(R.color.scan_middle));

        Scanner.with(this)
                .setFrameStrokeColor(Color.TRANSPARENT)
                .setFrameCornerWidth(BarCodeUtil.dp2px(this,2))
                .setFrameCornerColor(resources.getColor(R.color.color_more_blue))
                .setLaserBackground(R.drawable.img_scanner_grid)
                .setFrameStrategies(
                        NativeSdk.STRATEGY_ADAPTIVE_THRESHOLD_CLOSELY,
                        NativeSdk.STRATEGY_ADAPTIVE_THRESHOLD_REMOTELY)
//                .applyAllDecodeStrategiesInFrame()
                .setDetectorType(NativeSdk.PURE_ZXING)
                .setLaserLineMoveInterval(1500)
                .setTipText("")
                .setCoreThreadPoolSize(1)
                .setMaxThreadPoolSize(1)
//                .setContinuousScanTime(100)
//                .setFrameTopMargin(-BarCodeUtil.dp2px(this,200))
//                .setFrameSize(BarCodeUtil.dp2px(this,325), BarCodeUtil.dp2px(this,250))
                .setCaptureMode(ScanView.CAPTURE_MODE_TINY)
                .setScanMode(ScannerManager.ALL_MODE)
                .setTitle("我的扫一扫")
                .showAlbum(true)
                .setOnClickAlbumDelegate(new ScanActivityDelegate.OnClickAlbumDelegate() {
                    @Override
                    public void onClickAlbum(Activity activity) {
                        Intent albumIntent = new Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
                        activity.startActivityForResult(albumIntent, CODE_SELECT_IMAGE);
                    }

                    @Override
                    public void onSelectData(int requestCode, Intent data) {
                        if (requestCode == CODE_SELECT_IMAGE) {
                            selectPic(data);
                        }
                    }
                })
                .setOnScanResultDelegate(new ScanActivityDelegate.OnScanDelegate() {
                    @Override
                    public void onScanResult(ScanResult result) {
                        // 如果有回调，则必然有值,因为要避免AndroidX和support包的差异，所以没有默认的注解

//                        Intent intent = new Intent(MainActivity.this, DelegateActivity.class);
//                        intent.putExtra("result", result);
//                        startActivity(intent);

                        final String showContent = "format: " + result.getFormat().name()
                                + " light: " + result.getCameraLight()
                                + " zoom: " + result.getZoomTimes()
                                + " exposure: " + result.getExposureCompensation()
                                + " duration: " + result.getScanSuccessDuration()
                                + "  code: " + result.getContent();
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(MainActivity.this, showContent, Toast.LENGTH_SHORT).show();
                            }
                        });


                    }
                })
                .start();
    }

    public void openScanBox(View view) {
//        Intent intent = new Intent(this, CallBackTestActivity.class);
//        startActivity(intent);

        Bitmap bitmap = BitmapFactory.decodeResource(getResources(), R.drawable.test_boder_complex_8);
        CodeResult result = reader.read(bitmap);

        if (result == null) {
            Log.e("Scan >>> ", "no code");
            return;
        } else {
            Log.e("Scan >>> ", result.getText());
        }

        resultTxt.setText(result.getText());
    }

    public void chooseQRCodePic(View view) {
        Matisse.from(MainActivity.this)
                .choose(MimeType.ofAll())
                .countable(true)
                .maxSelectable(9)
                .gridExpectedSize(getResources().getDimensionPixelSize(R.dimen.grid_expected_size))
                .restrictOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED)
                .thumbnailScale(0.85f)
                .imageEngine(new GlideEngine())
                .forResult(REQUEST_CODE_CHOOSE);
    }

    public void customizeScanView(View v) {
        Intent intent = new Intent(this, NotFullScanActivity.class);
        startActivity(intent);
    }

    private void requestPermission() {
        AndPermission.with(this)
                .runtime()
                .permission(Permission.Group.CAMERA, Permission.Group.STORAGE)
                .onGranted(new Action<List<String>>() {
                    @Override
                    public void onAction(List<String> data) {

                    }
                })
                .onDenied(new Action<List<String>>() {
                    @Override
                    public void onAction(List<String> data) {

                    }
                })
                .start();
    }

    private void selectPic(Intent intent) {
        Uri selectImageUri = intent.getData();
        if (selectImageUri == null) {
            return;
        }
        String[] filePathColumn = {MediaStore.Images.Media.DATA};
        Cursor cursor = getContentResolver().query(selectImageUri, filePathColumn, null, null, null);
        if (cursor == null) {
            return;
        }
        cursor.moveToFirst();
        int columnIndex = cursor.getColumnIndex(filePathColumn[0]);
        String picturePath = cursor.getString(columnIndex);
        cursor.close();

        final Bitmap bitmap = BitmapFactory.decodeFile(picturePath);
        if (bitmap == null) {
            return;
        }

        handler.post(new Runnable() {
            @Override
            public void run() {
               final CodeResult result = reader.read(bitmap);
                if (result == null) {
                    Log.e("Scan >>> ", "no code");
                    return;
                } else {
                    Log.e("Scan >>> ", result.getText());
                }
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        resultTxt.setText(result.getText());
                    }
                });
            }
        });

    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_CODE_CHOOSE && resultCode == RESULT_OK) {
            List<Uri> selected = Matisse.obtainResult(data);
            if (selected.isEmpty()) {
                return;
            }
            Intent intent = new Intent(this, FindQRCodePositionActivity.class);
            intent.putExtra("picture_path", selected.get(0));
            startActivity(intent);
        }
    }

}
