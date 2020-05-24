package com.example.lfplayer.encoder;

/**
 * 音频编码器
 */
public interface IAudioEncoder {


    void encode(byte[] pcm, int readSize);

    void destroy();
}
