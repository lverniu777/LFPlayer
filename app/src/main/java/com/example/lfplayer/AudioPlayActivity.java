package com.example.lfplayer;

import android.app.Activity;
import android.content.Intent;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.text.TextUtils;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.example.lfplayer.utils.FileUtils;
import com.example.lfplayer.utils.Utils;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;

public class AudioPlayActivity extends AppCompatActivity {
    private static final int CHOOSE_FILE_REQUEST_CODE = 6;
    private final Object mLock = new RuntimeException();
    private AudioTrack mAudioTrack;
    private HandlerThread mHandlerThread;
    private Handler mHandler;

    static {
        System.loadLibrary("lfplayer");
    }

    @BindView(R.id.audio_file_path)
    TextView mAudioFilePath;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_play);
        ButterKnife.bind(this);
        mHandlerThread = new HandlerThread("audio play thread");
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper());
    }


    @OnClick({R.id.select_audio_file, R.id.play_audio})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.select_audio_file:
                Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                intent.setType("*/*");//无类型限制
                intent.addCategory(Intent.CATEGORY_OPENABLE);
                startActivityForResult(intent, CHOOSE_FILE_REQUEST_CODE);
                break;
            case R.id.play_audio:
                final String audioFilePath = mAudioFilePath.getText().toString();
                if (TextUtils.isEmpty(audioFilePath)) {
                    return;
                }
                nativePlayAudio(mAudioFilePath.getText().toString(), AudioPlayActivity.this);
                break;
        }
    }

    private native void nativePlayAudio(String audioFilePath, AudioPlayActivity audioPlayActivity);

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != CHOOSE_FILE_REQUEST_CODE) {
            return;
        }
        if (resultCode != Activity.RESULT_OK) {
            return;
        }
        if (data == null || data.getData() == null) {
            Toast.makeText(this, "没有选中文件", Toast.LENGTH_LONG).show();
        } else {
            final String filePath = FileUtils.INSTANCE.getPath(this, data.getData());
            mAudioFilePath.setText(filePath);
        }
    }

    private void audioPlay(byte[] audioData, int sampleRate, int channels) {
        if (mAudioTrack == null) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                mAudioTrack = new AudioTrack.Builder().build();
            } else {
                final int channelLayout = channels >= 2 ? AudioFormat.CHANNEL_OUT_STEREO : AudioFormat.CHANNEL_OUT_MONO;
                final int bufferSize = Math.max(
                        AudioTrack.getMinBufferSize(sampleRate, channelLayout, AudioFormat.ENCODING_PCM_FLOAT),
                        audioData.length);
                mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
                        sampleRate, channelLayout, AudioFormat.ENCODING_PCM_FLOAT,
                        bufferSize, AudioTrack.MODE_STREAM);
            }
            mAudioTrack.play();
        }
        mHandler.post(() -> {
            synchronized (mLock) {
                if (mAudioTrack == null) {
                    return;
                }
                mAudioTrack.write(audioData, 0, audioData.length);
            }
        });
        Utils.INSTANCE.log("sampleRate: " + sampleRate + " channels " + channels);

    }


    @Override
    protected void onDestroy() {
        super.onDestroy();
        synchronized (mLock) {
            if (mAudioTrack != null) {
                mAudioTrack.release();
            }
            mAudioTrack = null;
        }
    }
}
