package me.sam.czxing.loaders;

import android.graphics.Bitmap;

import androidx.annotation.NonNull;

import com.bumptech.glide.load.engine.bitmap_recycle.BitmapPool;
import com.bumptech.glide.load.resource.bitmap.BitmapTransformation;

import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;

import me.devilsen.czxing.code.NativeSdk;


public class FindQRCodeTransformer extends BitmapTransformation {
    private static final String ID = "com.bumptech.glide.transformations.FindQRCodeTransformer";

    @Override
    public Bitmap transform(BitmapPool pool, Bitmap toTransform, int outWidth, int outHeight) {
        Bitmap result = NativeSdk.getInstance().getQRCodeArea(toTransform);
        return result;
    }

    @Override
    public void updateDiskCacheKey(@NonNull MessageDigest messageDigest) {
        try {
            messageDigest.update(ID.getBytes(STRING_CHARSET_NAME));
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
    }

    @Override
    public boolean equals(Object o) {
        return o instanceof FindQRCodeTransformer;
    }

    @Override
    public int hashCode() {
        return ID.hashCode();
    }

}
