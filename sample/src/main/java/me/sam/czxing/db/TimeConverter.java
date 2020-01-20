package me.sam.czxing.db;

import androidx.room.TypeConverter;

import java.util.Date;

public class TimeConverter {

    @TypeConverter
    public static Date revertDate(long value) {
        return new Date(value);
    }

    @TypeConverter
    public static long converterDate(Date date) {
        return date == null ? null : date.getTime();
    }

}
