package com.example.lfplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import com.example.lfplayer.record.AudioRecordHelper;
import com.example.lfplayer.record.IAudioRecord;

public class AudioRecordActivity extends AppCompatActivity {

    private IAudioRecord mAudioRecord;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_record);
        mAudioRecord = new AudioRecordHelper();
        mAudioRecord.init();
        mAudioRecord.startRecord();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mAudioRecord.destroy();
    }
}
