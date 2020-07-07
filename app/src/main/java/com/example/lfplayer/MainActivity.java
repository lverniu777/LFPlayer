package com.example.lfplayer;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import org.libsdl.app.SDLActivity;

import butterknife.ButterKnife;
import butterknife.OnClick;

public class MainActivity extends AppCompatActivity {
    private static final int PERMISSION_REQUEST_CODE = 10086;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ButterKnife.bind(this);
        ActivityCompat.requestPermissions(this,
                new String[]{
                        Manifest.permission.WRITE_EXTERNAL_STORAGE,
                        Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.CAMERA,
                        Manifest.permission.RECORD_AUDIO
                },
                PERMISSION_REQUEST_CODE
        );
    }

    @OnClick({R.id.encode_preview_h264, R.id.sdl_player,
            R.id.audio_record, R.id.audio_play,
            R.id.surface_player})
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.encode_preview_h264:
                startActivity(new Intent(this, EncodeH264Activity.class));
                break;
            case R.id.sdl_player:
                startActivity(new Intent(this, SDLActivity.class));
                break;
            case R.id.audio_record:
                startActivity(new Intent(this, AudioRecordActivity.class));
                break;
            case R.id.audio_play:
                startActivity(new Intent(this, AudioPlayActivity.class));
                break;
            case R.id.surface_player:
                startActivity(new Intent(this, SurfacePlayerActivity.class));
                break;
        }
    }


    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode != PERMISSION_REQUEST_CODE) {
            finish();
            return;
        }
        for (int grantResult : grantResults) {
            if (grantResult != PackageManager.PERMISSION_GRANTED) {
                finish();
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }
}
