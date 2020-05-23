package com.example.lfplayer.utils;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;

import androidx.annotation.NonNull;

import org.jetbrains.annotations.NotNull;

import java.util.concurrent.Executor;

public class WorkHandler implements Executor {
    private HandlerThread mHandlerThread;
    private Handler mHandler;

    public WorkHandler() {

    }

    public void init(String threadName) {
        mHandlerThread = new HandlerThread(threadName);
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper());
    }

    @Override
    public void execute(@NotNull Runnable command) {
        mHandler.post(command);
    }

    public void stop() {
        mHandler.removeCallbacksAndMessages(null);
    }

    public void destroy() {
        mHandler.removeCallbacksAndMessages(null);
        mHandlerThread.quit();
    }
}
