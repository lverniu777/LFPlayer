package com.example.lfplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.PixelFormat;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class NativeOpenGLActivity extends AppCompatActivity {

    static {
        System.loadLibrary("lfplayer");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_native_open_gl);
        final SurfaceView surfaceView = findViewById(R.id.opengl_surface);
        surfaceView.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                new Thread(() -> {
                    nativeSurfaceCreated(holder.getSurface());
                }).start();
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });
    }

    private native void nativeSurfaceCreated(Surface surface);
}