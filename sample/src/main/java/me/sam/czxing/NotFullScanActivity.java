package me.sam.czxing;

import android.os.Bundle;
import android.widget.Toast;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import me.devilsen.czxing.Scanner;
import me.devilsen.czxing.ScannerManager;
import me.devilsen.czxing.code.BarcodeFormat;
import me.devilsen.czxing.code.NativeSdk;
import me.devilsen.czxing.util.BarCodeUtil;
import me.devilsen.czxing.view.ScanListener;
import me.devilsen.czxing.view.ScanView;

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
                .setFrameStrategies(NativeSdk.STRATEGY_ADAPTIVE_THRESHOLD)
                .setFrameSize(BarCodeUtil.dp2px(this, 335), BarCodeUtil.dp2px(this, 250))
                .setCaptureMode(CAPTURE_MODE_TINY)
                .setcontinuousScanTime(100)
                .setTipText("")
                .setScanMode(ScannerManager.ONE_D_MODE)
                .build();
        scannerView.applyScanOption(option);
        scannerView.setScanListener(new ScanListener() {
            @Override
            public void onScanSuccess(final String result, BarcodeFormat format) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(NotFullScanActivity.this, result, Toast.LENGTH_SHORT).show();
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
