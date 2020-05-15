package com.example.lfplayer.encoder;

import android.util.Log;

import com.example.lfplayer.FileUtils;

import java.io.File;

public class H264Encoder implements IEncoder {
    private static final String TAG = H264Encoder.class.getSimpleName();
    private final String mSavePath = FileUtils.INSTANCE.getROOT_DIR() + File.separator + System.currentTimeMillis() + ".h264";

    static {
        System.loadLibrary("avcodec");
        System.loadLibrary("x264");
        System.loadLibrary("lfplayer");
    }


    @Override
    public void encode(byte[] frame, int width, int height) {
//        Log.e(TAG, this + " encode: " + frame.length + " width: " + width + " height: " + height);
        encodeH264(frame, width, height, mSavePath);
    }

    private native void encodeH264(byte[] frame, int width, int height, String filePath);

}
