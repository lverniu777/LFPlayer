package com.example.lfplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import com.example.lfplayer.encoder.H264AACEncoder;
import com.example.lfplayer.encoder.IVideoEncoder;
import com.example.lfplayer.utils.FileUtils;
import com.example.lfplayer.view.CameraSurfaceView;

import java.io.File;

import butterknife.BindView;
import butterknife.ButterKnife;

public class EncodeH264Activity extends AppCompatActivity {

    @BindView(R.id.camera_surface_view)
    CameraSurfaceView mCameraSurfaceView;

    private IVideoEncoder mEncoder;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_encode_h264);
        ButterKnife.bind(this);
        mEncoder = new H264AACEncoder(FileUtils.INSTANCE.getROOT_DIR() + File.separator + System.currentTimeMillis() + ".h264");
        mCameraSurfaceView.setFrameListener(mEncoder::encode);
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (mCameraSurfaceView != null) {
            mCameraSurfaceView.onPause();
        }
        if (mEncoder != null) {
            mEncoder.flush();
        }
    }
}
