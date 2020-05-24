package com.example.lfplayer.utils;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;

import androidx.annotation.IntDef;
import androidx.annotation.NonNull;

import org.jetbrains.annotations.NotNull;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.concurrent.Executor;

public class WorkHandler implements Executor {
    private static final int RUNNING = 82;
    private static final int STOP = 223;
    private HandlerThread mHandlerThread;
    private Handler mHandler;
    private volatile int mState = STOP;

    @IntDef({RUNNING, STOP})
    @Retention(RetentionPolicy.SOURCE)
    @interface Sate {
    }

    public WorkHandler() {

    }

    public void init(String threadName) {
        mHandlerThread = new HandlerThread(threadName);
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper());
    }

    public void start() {
        setState(RUNNING);
    }


    @Override
    public void execute(@NotNull Runnable command) {
        if (mState != RUNNING) {
            return;
        }
        mHandler.post(command);
    }


    public void stop() {
        mHandler.removeCallbacksAndMessages(null);
        setState(STOP);
        Utils.INSTANCE.log("stop audio record");
    }


    private void setState(@Sate int state) {
        mState = state;
    }

    public void destroy() {
        mHandler.removeCallbacksAndMessages(null);
        mHandlerThread.quit();
    }

}
