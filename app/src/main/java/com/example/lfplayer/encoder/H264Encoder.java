package com.example.lfplayer.encoder;

import android.os.Handler;
import android.os.HandlerThread;

import com.example.lfplayer.FileUtils;

import java.io.File;

public class H264Encoder implements IEncoder {
    private static final String TAG = H264Encoder.class.getSimpleName();
    private final String mSavePath = FileUtils.INSTANCE.getROOT_DIR() + File.separator + System.currentTimeMillis() + ".h264";
    private volatile HandlerThread mHandlerThread;
    private volatile Handler mHandler;
    private final Object mEncodeLock = new Object();
    private byte[] mTempBuffer;


    static {
        System.loadLibrary("avcodec");
        System.loadLibrary("x264");
        System.loadLibrary("lfplayer");
    }

    public H264Encoder() {
        nativeInit(mSavePath);
        mHandlerThread = new HandlerThread("encode h264 thread");
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper());
    }

    private native void nativeInit(String mSavePath);


    @Override
    public void encode(byte[] frame, int width, int height) {
        final Handler handler = mHandler;
        if (handler == null) {
            return;
        }
        handler.post(() -> encodeH264(rotateYUV420Degree90(frame, width, height), height, width, mSavePath));
    }

    @Override
    public void flush() {
        final Handler handler = mHandler;
        if (handler == null) {
            return;
        }
        final HandlerThread handlerThread = mHandlerThread;
        if (handlerThread == null) {
            return;
        }
        handler.post(() -> {
            nativeFlush();
            handler.removeCallbacksAndMessages(null);
            handlerThread.quit();
            mHandler = null;
            mHandlerThread = null;
            synchronized (mEncodeLock) {
                mTempBuffer = null;
            }
        });


    }

    //相机预览数据需要进行方向旋转
    private byte[] rotateYUV420Degree90(byte[] data, int imageWidth, int imageHeight) {
        final byte[] nv21;
        if (data == null) {
            return new byte[1];
        }
        synchronized (mEncodeLock) {
            if (mTempBuffer == null) {
                mTempBuffer = new byte[imageWidth * imageHeight * 3 / 2];
            }
            nv21 = mTempBuffer;
        }
        // Rotate the Y luma
        int i = 0;
        for (int x = 0; x < imageWidth; x++) {
            for (int y = imageHeight - 1; y >= 0; y--) {
                nv21[i] = data[y * imageWidth + x];
                i++;
            }
        }
        // Rotate the U and V color components
        i = imageWidth * imageHeight * 3 / 2 - 1;
        for (int x = imageWidth - 1; x > 0; x = x - 2) {
            for (int y = 0; y < imageHeight / 2; y++) {
                nv21[i] = data[(imageWidth * imageHeight) + (y * imageWidth) + x];
                i--;
                nv21[i] = data[(imageWidth * imageHeight) + (y * imageWidth) + (x - 1)];
                i--;
            }
        }
        return nv21;
    }

    private native void nativeFlush();

    private native void encodeH264(byte[] frame, int width, int height, String filePath);

}
