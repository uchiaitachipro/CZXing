package me.sam.czxing.db.entities;

import androidx.room.Entity;
import androidx.room.PrimaryKey;

import java.util.Date;

@Entity(tableName = "profile_time")
public class ProfileModel {

    @PrimaryKey
    public int id;

    public long total;

    public long thresholdTime;

    public long decodeTime;

    public int type;

    public Date currentTime;

}
