//
// Created by 79852 on 2020/5/14.
//
#include <jni.h>
#include <player/lfplayerutils.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <rtmp/rtmp.h>
}
static int base = 0;
static FILE *openFile = NULL;
AVCodecContext *avCodecContext = NULL;
AVFrame *avFrame = NULL;
AVPacket *avPacket = NULL;

static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
                   FILE *outfile) {
    int ret;
    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        LOG("avcodec_send_frame: failed %s", av_err2str(ret));
        return;
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            LOG("avcodec_receive_packet: %s", av_err2str(ret));
            return;
        } else if (ret < 0) {
            LOG("avcodec_receive_packet: failed %s", av_err2str(ret));
            return;
        }
        int wirteSize = fwrite(pkt->data, 1, pkt->size, outfile);
        LOG("write size %d pts %d", wirteSize, pkt->pts);
        av_packet_unref(pkt);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_lfplayer_encoder_H264AACEncoder_nativeInit(JNIEnv *env, jobject thiz,
                                                            jstring m_save_path) {
    LOG("nativeInit");
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_lfplayer_encoder_H264AACEncoder_encodeH264(JNIEnv *env, jobject thiz,
                                                            jbyteArray frameArray, jint width,
                                                            jint height,
                                                            jstring file_path) {
    if (!avCodecContext) {
        const char *filePath = env->GetStringUTFChars(file_path, NULL);
        base = 0;
        openFile = fopen(filePath, "wb+");
        AVCodec *codec = avcodec_find_encoder_by_name("libx264");
        avCodecContext = avcodec_alloc_context3(codec);
        avCodecContext->bit_rate = 6300000;
        avCodecContext->width = width;
        avCodecContext->height = height;
        avCodecContext->gop_size = 250;
        avCodecContext->keyint_min = 25;
        avCodecContext->max_b_frames = 3;
        avCodecContext->has_b_frames = 1;
        avCodecContext->refs = 3;
        avCodecContext->profile = FF_PROFILE_H264_HIGH_444;
        avCodecContext->level = 50;
        avCodecContext->time_base = (AVRational) {1, 25};
        avCodecContext->framerate = (AVRational) {25, 1};
        avCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
        avcodec_open2(avCodecContext, avCodecContext->codec, NULL);
        avFrame = av_frame_alloc();
        avFrame->format = avCodecContext->pix_fmt;
        avFrame->width = avCodecContext->width;
        avFrame->height = avCodecContext->height;
        av_frame_get_buffer(avFrame, 32);
        av_frame_make_writable(avFrame);
        avPacket = av_packet_alloc();
        av_init_packet(avPacket);
    }
    jbyte *nv21 = env->GetByteArrayElements(frameArray, 0);
    int x, y;
    int bIndex = 0;
    for (y = 0; y < avCodecContext->height; y++) {
        for (x = 0; x < avCodecContext->width; x++) {
            avFrame->data[0][y * avFrame->linesize[0] + x] = nv21[bIndex++];
        }
    }
    for (y = 0; y < avCodecContext->height / 2; y++) {
        for (x = 0; x < avCodecContext->width / 2; x++) {
            avFrame->data[2][y * avFrame->linesize[2] + x] = nv21[bIndex++];
            avFrame->data[1][y * avFrame->linesize[1] + x] = nv21[bIndex++];
        }
    }
    avFrame->pts = base++;
    encode(avCodecContext, avFrame, avPacket, openFile);
    env->ReleaseByteArrayElements(frameArray, nv21, JNI_ABORT);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_lfplayer_encoder_H264AACEncoder_nativeFlush(JNIEnv *env, jobject thiz) {
    if (avPacket && avCodecContext && openFile) {
        encode(avCodecContext, NULL, avPacket, openFile);
        fclose(openFile);
        openFile = NULL;
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
        av_packet_free(&avPacket);
        av_frame_free(&avFrame);
        LOG("nativeFlush")
    }
    base = 0;
}
















extern "C"
JNIEXPORT void JNICALL
Java_com_example_lfplayer_encoder_H264AACEncoder_nativeEncodeAAC(JNIEnv *env, jobject thiz,
                                                                 jbyteArray pcm, jint read_size,
                                                                 jstring m_save_path) {
    if (!openFile) {
        const char *fileName = env->GetStringUTFChars(m_save_path, NULL);
        openFile = fopen(fileName, "wb+");
        AVCodec *audioCodec = avcodec_find_encoder_by_name("libfdk_aac");
        if (!audioCodec) {
            LOG("not found audio codec");
            return;
        }
        avCodecContext = avcodec_alloc_context3(audioCodec);
        if (!avCodecContext) {
            LOG("avcodec_alloc_context3 failed");
            return;
        }
        avCodecContext->profile = FF_PROFILE_AAC_LOW;
        avCodecContext->bit_rate = 128000;
        avCodecContext->sample_rate = 44100;
        avCodecContext->channels = 2;
        avCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
        avCodecContext->sample_fmt = AV_SAMPLE_FMT_S16;
        int ret = avcodec_open2(avCodecContext, audioCodec, NULL);
        if (ret) {
            LOG("avcodec_open2 failed %s", av_err2str(ret));
            return;
        }
        avFrame = av_frame_alloc();
        if (!avFrame) {
            LOG("av_frame_alloc failed");
            return;
        }
        avFrame->sample_rate = avCodecContext->sample_rate;
        avFrame->format = avCodecContext->sample_fmt;
        avFrame->channels = avCodecContext->channels;
        avFrame->channel_layout = avCodecContext->channel_layout;
        avFrame->nb_samples = avCodecContext->frame_size;
        ret = av_frame_get_buffer(avFrame, 0);
        if (ret) {
            LOG("av_frame_get_buffer failed %s", av_err2str(ret));
            return;
        }
        av_frame_make_writable(avFrame);
        avPacket = av_packet_alloc();
        av_init_packet(avPacket);
    }
    jbyte *pcmBytes = env->GetByteArrayElements(pcm, NULL);
    const int length = env->GetArrayLength(pcm);
    int index = 0;
    while (index < length) {
        for (int i = 0; i < avFrame->linesize[0] && i < length && index < length; i++) {
            avFrame->data[0][i] = static_cast<uint8_t>(pcmBytes[index++]);
        }
        encode(avCodecContext, avFrame, avPacket, openFile);
    }
    env->ReleaseByteArrayElements(pcm, pcmBytes, JNI_ABORT);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lfplayer_encoder_H264AACEncoder_nativeDestroy(JNIEnv *env, jobject thiz) {
    if (openFile) {
        encode(avCodecContext, NULL, avPacket, openFile);
        av_packet_free(&avPacket);
        av_frame_free(&avFrame);
        avcodec_free_context(&avCodecContext);
        fclose(openFile);
        openFile = NULL;
    }
}