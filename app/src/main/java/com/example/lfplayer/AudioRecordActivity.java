package com.example.lfplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;

import com.example.lfplayer.record.AudioRecordHelper;
import com.example.lfplayer.record.IAudioRecord;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.internal.Utils;

public class AudioRecordActivity extends AppCompatActivity {

    private IAudioRecord mAudioRecord;
    @BindView(R.id.audio_record_start)
    View mAudioRecordStart;

    @BindView(R.id.audio_record_end)
    View mAudioRecordEnd;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio_record);
        ButterKnife.bind(this);
        mAudioRecord = new AudioRecordHelper();
        mAudioRecord.init();
    }


    @OnClick({R.id.audio_record_start, R.id.audio_record_end})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.audio_record_start:
                mAudioRecord.startRecord();
                break;
            case R.id.audio_record_end:
                mAudioRecord.stopRecord();
                break;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mAudioRecord.destroy();
    }
}
