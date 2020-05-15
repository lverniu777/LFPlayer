package com.example.lfplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import com.example.lfplayer.encoder.H264Encoder;
import com.example.lfplayer.view.CameraSurfaceView;

import butterknife.BindView;
import butterknife.ButterKnife;

public class EncodeH264Activity extends AppCompatActivity {

    @BindView(R.id.camera_surface_view)
    CameraSurfaceView mCameraSurfaceView;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_encode_h264);
        ButterKnife.bind(this);
        mCameraSurfaceView.setFrameListener(new H264Encoder()::encode);
    }

    @Override
    protected void onPause() {
        super.onPause();
        if(mCameraSurfaceView != null) {
            mCameraSurfaceView.onPause();
        }
    }
}
