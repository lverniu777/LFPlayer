package com.example.lfplayer.encoder;

public interface IEncoder {
    void encode(byte[] frame, int width, int height);

    void flush();
}
