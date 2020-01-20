package me.sam.czxing.activitys;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.Toast;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import me.devilsen.czxing.code.BarcodeFormat;
import me.devilsen.czxing.code.BarcodeReader;
import me.devilsen.czxing.code.BarcodeWriter;
import me.devilsen.czxing.code.CodeResult;
import me.devilsen.czxing.util.BarCodeUtil;
import me.sam.czxing.R;

/**
 * desc :
 * date : 2019-07-22 18:25
 *
 * @author : dongSen
 */
public class WriteCodeActivity extends AppCompatActivity implements View.OnClickListener {

    private ImageView qrcodeImage;
    private ImageView qrcodeLogoImage;
    private ImageView barcodeImage;
    private ImageView barcodeColorImage;
    private BarcodeWriter writer;
    private BarcodeReader reader;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_write_code);

        qrcodeImage = findViewById(R.id.image_view_qr_code_1);
        qrcodeLogoImage = findViewById(R.id.image_view_qr_code_2);
        barcodeImage = findViewById(R.id.image_view_bar_code_1);
        barcodeColorImage = findViewById(R.id.image_view_bar_code_2);

        qrcodeImage.setOnClickListener(this);
        qrcodeLogoImage.setOnClickListener(this);
        barcodeImage.setOnClickListener(this);
        barcodeColorImage.setOnClickListener(this);

        writer = new BarcodeWriter();
        reader = BarcodeReader.getInstance();

        writeQrCode();
        writeBarCode();
    }

    /**
     * 生成二维码与识别二维码都是耗时操作，请务必放在子线程中执行，这里是为了演示的简洁才采用这种写法
     */
    private void writeQrCode() {
        Bitmap bitmap1 = writer.write("Hello World",
                BarCodeUtil.dp2px(this, 150),
                Color.BLACK);

        Bitmap logoBitmap = BitmapFactory.decodeResource(getResources(), R.drawable.ic_avatar);
        Bitmap bitmap2 = writer.write("你好，こんにちは，여보세요",
                BarCodeUtil.dp2px(this, 150),
                Color.parseColor("#2196F3"),
                logoBitmap);

        if (bitmap1 != null) {
            qrcodeImage.setImageBitmap(bitmap1);
        }

        if (bitmap2 != null) {
            qrcodeLogoImage.setImageBitmap(bitmap2);
        }
    }

    private void writeBarCode() {
        Bitmap barcodeBitmap = writer.writeBarCode("Hello CZXing",
                BarCodeUtil.dp2px(this, 150), BarCodeUtil.dp2px(this, 80));

        Bitmap barcodeBitmap2 = writer.writeBarCode("20190729",
                BarCodeUtil.dp2px(this, 150), BarCodeUtil.dp2px(this, 80),
                Color.parseColor("#2196F3"));

        if (barcodeBitmap != null) {
            barcodeImage.setImageBitmap(barcodeBitmap);
        }

        if (barcodeBitmap2 != null) {
            barcodeColorImage.setImageBitmap(barcodeBitmap2);
        }
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        switch (id) {
            case R.id.image_view_qr_code_1:
                read(getBitmap(qrcodeImage));
                break;
            case R.id.image_view_qr_code_2:
                read(getBitmap(qrcodeLogoImage));
                break;
            case R.id.image_view_bar_code_1:
                read(getBitmap(barcodeImage));
                break;
            case R.id.image_view_bar_code_2:
                read(getBitmap(barcodeColorImage));
                break;
        }
    }

    private Bitmap getBitmap(ImageView imageView){
        BitmapDrawable drawable = (BitmapDrawable) imageView.getDrawable();
        return drawable.getBitmap();
    }

    public void read(Bitmap bitmap) {
        CodeResult result = reader.read(bitmap);
        if (result != null) {
            Log.d("read code", result.getText() + " format " + result.getFormat());
            Toast.makeText(this, result.getText(), Toast.LENGTH_SHORT).show();
        }
    }
}
