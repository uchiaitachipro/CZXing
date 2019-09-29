package me.sam.czxing;

import android.net.Uri;
import android.os.Bundle;
import android.widget.ImageView;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.bumptech.glide.Glide;
import com.bumptech.glide.load.engine.DiskCacheStrategy;

import me.sam.czxing.loaders.FindQRCodeTransformer;

import static com.bumptech.glide.load.EncodeStrategy.SOURCE;

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
                .diskCacheStrategy(DiskCacheStrategy.NONE) // override default RESULT cache and apply transform always
                .skipMemoryCache(true) // do not reuse the transformed result while running
                .transform(new FindQRCodeTransformer())
                .into(imageView);
    }
}
