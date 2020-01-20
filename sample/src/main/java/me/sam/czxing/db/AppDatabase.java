package me.sam.czxing.db;

import androidx.room.Database;
import androidx.room.RoomDatabase;
import androidx.room.TypeConverters;

import me.sam.czxing.db.daos.ProfileModelDao;
import me.sam.czxing.db.entities.ProfileModel;

@Database(entities = {ProfileModel.class}, version = 1,exportSchema = false)
@TypeConverters({TimeConverter.class})
public abstract class AppDatabase extends RoomDatabase {
    public abstract ProfileModelDao profileModelDao();
}
