package com.example.lfplayer.record;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Handler;
import android.os.HandlerThread;

import com.example.lfplayer.utils.Utils;
import com.example.lfplayer.utils.WorkHandler;


public class AudioRecordHelper implements IAudioRecord {
    private static final int SAMPLE_RATE = 44100;
    private static final int CHANNEL = AudioFormat.CHANNEL_IN_STEREO;
    private static final int AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT;
    private static final int AUDIO_SOURCE = MediaRecorder.AudioSource.MIC;
    private volatile byte[] mBuffer;
    private WorkHandler mWorkHandler;
    private AudioRecord mAudioRecorder;

    public void init() {
        mWorkHandler = new WorkHandler();
        mWorkHandler.init("audio record thread");
        final int bufferSize = AudioRecord.getMinBufferSize(SAMPLE_RATE, CHANNEL, AUDIO_FORMAT);
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {
            mAudioRecorder = new AudioRecord.Builder()
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
            mAudioRecorder = new AudioRecord(AUDIO_SOURCE, SAMPLE_RATE, CHANNEL, AUDIO_FORMAT, bufferSize);
        }
        mBuffer = new byte[bufferSize];
    }


    @Override
    public void startRecord() {
        if (mAudioRecorder == null) {
            throw new IllegalStateException("not init");
        }
        mAudioRecorder.startRecording();
        mWorkHandler.execute(new Runnable() {
            @Override
            public void run() {
                final int readSize = mAudioRecorder.read(mBuffer, 0, mBuffer.length);
                Utils.INSTANCE.log("audio record read size: " + readSize);
                mWorkHandler.execute(this);
            }
        });
    }

    @Override
    public void stopRecord() {
        if (mAudioRecorder == null) {
            return;
        }
        mWorkHandler.stop();
    }

    @Override
    public void destroy() {
        if (mAudioRecorder == null) {
            return;
        }
        mWorkHandler.destroy();
        mAudioRecorder.release();
        mAudioRecorder = null;
    }


}
