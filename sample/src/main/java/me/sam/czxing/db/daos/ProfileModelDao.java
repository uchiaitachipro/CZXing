package me.sam.czxing.db.daos;

import androidx.room.Dao;
import androidx.room.Delete;
import androidx.room.Insert;
import androidx.room.OnConflictStrategy;
import androidx.room.Query;

import java.util.List;

import io.reactivex.Flowable;
import me.sam.czxing.db.entities.ProfileModel;

@Dao
public interface ProfileModelDao {

    @Query("SELECT * FROM profile_time WHERE type = :type")
    Flowable<List<ProfileModel>> getProfileModels(int type);

    @Query("SELECT * FROM profile_time WHERE type = :type AND currentTime = :startTime")
    Flowable<List<ProfileModel>> getProfileModels(int type,long startTime);

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    void insertProfileData(ProfileModel... users);

    @Insert
    void insertProfileData(List<ProfileModel> friends);

    @Delete
    void deleteProfileData(ProfileModel... users);

    @Query("DELETE FROM profile_time")
    void deleteAll();
}
