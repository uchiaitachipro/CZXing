package me.sam.czxing.activitys;

import android.os.Bundle;
import android.widget.Toast;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import me.devilsen.czxing.ScanResult;
import me.devilsen.czxing.Scanner;
import me.devilsen.czxing.ScannerManager;
import me.devilsen.czxing.util.BarCodeUtil;
import me.devilsen.czxing.view.ScanListener;
import me.devilsen.czxing.view.ScanView;
import me.sam.czxing.R;

import static me.devilsen.czxing.view.ScanView.CAPTURE_MODE_TINY;

/**
 * Created by uchia on 2019-10-20
 **/
public class NotFullScanActivity extends AppCompatActivity {

    private ScanView scannerView;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test_not_full_scan_view);
        initScanView();
    }

    private void initScanView() {
        scannerView = findViewById(R.id.scanner_view);
        ScannerManager.ScanOption option = Scanner.with(this)
//                .setFrameStrategies(NativeSdk.STRATEGY_ADAPTIVE_THRESHOLD_CLOSELY)
                .setFrameSize(BarCodeUtil.dp2px(this, 335), BarCodeUtil.dp2px(this, 250))
                .setCaptureMode(CAPTURE_MODE_TINY)
                .setContinuousScanTime(100)
                .setPotentialAreaStrategy(ScannerManager.FIND_POTENTIAL_AREA_FOCUS)
                .setScanBoxOffset(0)
                .setTipText("")
                .setScanMode(ScannerManager.ONE_D_MODE)
                .build();
        scannerView.applyScanOption(option);
        scannerView.setScanListener(new ScanListener() {
            @Override
            public void onScanSuccess(final ScanResult result) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(NotFullScanActivity.this,
                                "cameraLight: " + result.getCameraLight() +
                                        " result " + result.getContent() +
                                        " zoom: " + result.getZoomTimes(), Toast.LENGTH_SHORT).show();
                    }
                });
            }

            @Override
            public void onScanFail() {

            }

            @Override
            public void onOpenCameraError() {

            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        scannerView.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        scannerView.onPause();
    }
}
