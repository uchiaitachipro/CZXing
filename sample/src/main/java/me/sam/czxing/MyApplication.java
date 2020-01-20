package me.sam.czxing;

import android.app.Application;
import android.content.Context;

import com.tencent.bugly.crashreport.CrashReport;

import me.sam.czxing.db.DatabaseHelper;

/**
 * desc :
 * date : 2019-06-25
 *
 * @author : dongSen
 */
public class MyApplication extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        DatabaseHelper.getInstance().init(this);
    }
}
