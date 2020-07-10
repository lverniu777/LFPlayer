package com.example.lfplayer;

import android.graphics.PixelFormat;
import android.os.Bundle;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;

public class SurfacePlayerActivity extends BaseSelectMediaFileActivity {

    static {
        System.loadLibrary("lfplayer");
    }

    @BindView(R.id.selected_file_path)
    TextView mVideoPath;
    @BindView(R.id.surface_view)
    SurfaceView mSurfaceView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_surface_player);
        ButterKnife.bind(this);
        mSurfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                holder.setFormat(PixelFormat.TRANSLUCENT);
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {

            }
        });
        mSurfaceView.setZOrderOnTop(false);
    }

    @OnClick({R.id.surface_play, R.id.select_video_file})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.surface_play:
                new Thread(() ->
                        nativePlayVideo(
                                mVideoPath.getText().toString(),
                                mSurfaceView.getHolder().getSurface()
                        )).start();
                break;
            case R.id.select_video_file:
                requestSelectFile();
                break;
        }
    }

    private native void nativePlayVideo(String toString, Surface surface);


    @Override
    protected void onPathSelected(String path) {
        mVideoPath.setText(path);
    }
}