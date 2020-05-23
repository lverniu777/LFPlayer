package com.example.lfplayer.encoder;

public interface IVideoEncoder {
    void encode(byte[] frame, int width, int height);

    void flush();
}
