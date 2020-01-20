package me.sam.czxing.db;

import android.content.Context;

import androidx.room.Room;

import java.util.List;

import io.reactivex.Flowable;
import io.reactivex.functions.Consumer;
import io.reactivex.schedulers.Schedulers;
import me.devilsen.czxing.code.NativeSdk;
import me.sam.czxing.db.entities.ProfileModel;

public class DatabaseHelper {

    private static volatile DatabaseHelper sInstance;

    private AppDatabase database;

    private DatabaseHelper(){}

    public static DatabaseHelper getInstance(){
        if (sInstance == null){
            synchronized (DatabaseHelper.class){
                if (sInstance == null){
                    sInstance = new DatabaseHelper();
                }
            }
        }
        return sInstance;
    }

    public void init(Context context){
        database = Room.databaseBuilder(context,
                AppDatabase.class, "qrcode-optimize").build();
    }

    public AppDatabase getDatabase(){
        if (database == null){
            throw new IllegalStateException("not inited");
        }
        return database;
    }

    public Flowable<List<ProfileModel>> getAllZXingRecord(){
        return getDatabase().profileModelDao().getProfileModels(NativeSdk.DETECTOR_ZXING);
    }

    public Flowable<List<ProfileModel>> getAllZBarRecord(){
        return getDatabase().profileModelDao().getProfileModels(NativeSdk.DETECTOR_ZBAR);
    }

    public Flowable<List<ProfileModel>> getAllMixedRecord(){
        return getDatabase().profileModelDao().getProfileModels(NativeSdk.DETECTOR_ALL);
    }

    public void deleteZXingRecord(){
        deleteProfileRecordByType(NativeSdk.DETECTOR_ZXING);
    }

    public void deleteZBarRecord(){
        deleteProfileRecordByType(NativeSdk.DETECTOR_ZBAR);
    }

    public void deleteMixedDecordRecord(){
        deleteProfileRecordByType(NativeSdk.DETECTOR_ALL);
    }

    public void deleteAllScanRecord(){
        getDatabase().profileModelDao().deleteAll();
    }

    private void deleteProfileRecordByType(final int type){
        Flowable.just(1)
                .subscribeOn(Schedulers.io())
                .subscribe(new Consumer<Integer>() {
                    @Override
                    public void accept(Integer integer) throws Exception {
                        getDatabase().profileModelDao().deleteByType(type);
                    }
                });
    }

}
