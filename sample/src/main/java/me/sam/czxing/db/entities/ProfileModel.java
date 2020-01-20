package me.sam.czxing.db.entities;

import androidx.room.Entity;
import androidx.room.PrimaryKey;

import java.util.Date;

@Entity(tableName = "profile_time")
public class ProfileModel {

    @PrimaryKey(autoGenerate = true)
    public int id;

    public long total;

    public long thresholdTime;

    public long reLocationTime;

    public long zxingTime;

    public long zbarTime;

    public int type;

    public boolean isSuccess;

    public Date currentTime;

}
