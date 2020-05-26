package com.example.lfplayer.record;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

import com.example.lfplayer.encoder.H264AACEncoder;
import com.example.lfplayer.encoder.IAudioEncoder;
import com.example.lfplayer.utils.FileUtils;
import com.example.lfplayer.utils.Utils;
import com.example.lfplayer.utils.WorkHandler;

import java.io.File;


public class AudioRecordHelper implements IAudioRecord {
    private static final int SAMPLE_RATE = 44100;
    private static final int CHANNEL = AudioFormat.CHANNEL_IN_STEREO;
    private static final int AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT;
    private static final int AUDIO_SOURCE = MediaRecorder.AudioSource.MIC;
    private volatile byte[] mBuffer;
    private WorkHandler mWorkHandler;
    private AudioRecord mAudioRecord;
    private IAudioEncoder mAudioEncoder;


    public void init() {
        mAudioEncoder = new H264AACEncoder(FileUtils.INSTANCE.getROOT_DIR() + File.separator + System.currentTimeMillis() + ".aac");
        mWorkHandler = new WorkHandler();
        mWorkHandler.init("audio record thread");
        final int bufferSize = AudioRecord.getMinBufferSize(SAMPLE_RATE, CHANNEL, AUDIO_FORMAT);
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
            mAudioRecord = new AudioRecord.Builder()
                    .setAudioSource(AUDIO_SOURCE)
                    .setAudioFormat(
                            new AudioFormat.Builder()
                                    .setSampleRate(SAMPLE_RATE)
                                    .setEncoding(AUDIO_FORMAT)
                                    .setChannelMask(CHANNEL)
                                    .build()
                    )
                    .setBufferSizeInBytes(bufferSize)
                    .build();
        } else {
            mAudioRecord = new AudioRecord(AUDIO_SOURCE, SAMPLE_RATE, CHANNEL, AUDIO_FORMAT, bufferSize);
        }
        mBuffer = new byte[bufferSize];
    }


    @Override
    public void startRecord() {
        if (mAudioRecord == null) {
            throw new IllegalStateException("not init");
        }
        if (mAudioRecord.getRecordingState() == AudioRecord.RECORDSTATE_RECORDING) {
            return;
        }
        mAudioRecord.startRecording();
        mWorkHandler.start();
        mWorkHandler.execute(new Runnable() {
            @Override
            public void run() {
                if (mAudioRecord.getRecordingState() != AudioRecord.RECORDSTATE_RECORDING) {
                    return;
                }
                final int readSize = mAudioRecord.read(mBuffer, 0, mBuffer.length);
                mAudioEncoder.encode(mBuffer, readSize);
                Utils.INSTANCE.log("audio record read size: " + readSize);
                mWorkHandler.execute(this);
            }
        });
    }

    @Override
    public void stopRecord() {
        if (mAudioRecord == null) {
            return;
        }
        mAudioRecord.stop();
        mWorkHandler.stop();
    }

    @Override
    public void destroy() {
        if (mAudioRecord == null) {
            return;
        }
        mWorkHandler.destroy();
        mAudioRecord.release();
        mAudioEncoder.destroy();
        mAudioRecord = null;
        mBuffer = null;
    }


}
