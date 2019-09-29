package me.sam.czxing;

import android.net.Uri;
import android.os.Bundle;
import android.view.Window;
import android.widget.ImageView;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.bumptech.glide.Glide;

import me.sam.czxing.loaders.FindQRCodeTransformer;

public class FindQRCodePositionActivity extends AppCompatActivity {

    private ImageView imageView;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_find_qr);
        getSupportActionBar().hide();//这行代码必须写在setContentView()方法的后面
        initView();
    }

    private void initView(){
        imageView = findViewById(R.id.image);
        Uri uri = getIntent().getParcelableExtra("picture_path");
        if (uri == null){
            return;
        }
        Glide.with(this)
                .load(uri)
                .transform(new FindQRCodeTransformer())
                .into(imageView);
    }
}
