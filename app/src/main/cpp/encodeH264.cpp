//
// Created by 79852 on 2020/5/14.
//
#include <jni.h>
#include <player/lfplayerutils.h>
#include <cstdio>

static FILE *openFile;
extern "C" {
#include <libavcodec/avcodec.h>
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lfplayer_encoder_H264Encoder_nativeInit(JNIEnv *env, jobject thiz,
                                                         jstring m_save_path) {
    const char *filePath = env->GetStringUTFChars(m_save_path, NULL);
    openFile = fopen(filePath, "wb+");
    if (!openFile) {
        LOG("fopen error");
        return;
    }

}

void openCodec(int width, int height, AVCodecContext **pAvCodecContext) {
    AVCodec *avCodec = avcodec_find_encoder_by_name("libx264");
    if (avCodec == NULL) {
        LOG("not find avcodec");
        return;
    }
    AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);
    if (!avCodecContext) {
        LOG("avcodec_alloc_context3 failed");
        return;
    }
    *pAvCodecContext = avCodecContext;
    avCodecContext->profile = FF_PROFILE_H264_CAVLC_444;
    avCodecContext->level = 50;
    avCodecContext->gop_size = 250;
    avCodecContext->keyint_min = 25;

    avCodecContext->max_b_frames = 3;
    avCodecContext->has_b_frames = 1;
    avCodecContext->refs = 3;
    avCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    avCodecContext->bit_rate = 6300000;
    avCodecContext->time_base = (AVRational) {1, 25};
    avCodecContext->framerate = (AVRational) {25, 1};
    int ret;
    if ((ret = avcodec_open2(avCodecContext, avCodec, NULL))) {
        LOG("avcodec_open2 failed %s ", av_err2str(ret));
        return;
    }
}

AVFrame *createFrame(int width, int height) {
    AVFrame *avFrame = av_frame_alloc();
    if (!avFrame) {
        LOG("av_frame_alloc failed");
        return NULL;
    }
    avFrame->width = width;
    avFrame->height = height;
    //libx264只支持YUV420P的编码格式
    avFrame->format = AV_PIX_FMT_YUV420P;
    int ret = av_frame_get_buffer(avFrame, 32);
    if (ret) {
        LOG("av_frame_get_buffer failed: %s", av_err2str(ret));
        av_frame_free(&avFrame);
        return NULL;
    }

}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_lfplayer_encoder_H264Encoder_encodeH264(JNIEnv *env, jobject thiz,
                                                         jbyteArray frame, jint width, jint height,
                                                         jstring file_path) {
    if (!openFile) {
        Java_com_example_lfplayer_encoder_H264Encoder_nativeInit(env, thiz, file_path);
    }
    jbyte *bytes = env->GetByteArrayElements(frame, 0);
    int chars_len = env->GetArrayLength(frame);
    char *chars = new char[chars_len];
    memset(chars, 0, chars_len);
    memcpy(chars, bytes, chars_len);
    fwrite(bytes, 1, chars_len, openFile);
    env->ReleaseByteArrayElements(frame, bytes, 0);
//    AVCodecContext *avCodecContext = NULL;
//    openCodec(width, height, &avCodecContext);
//
//    AVFrame *avFrame = createFrame(width, height);
//
//    AVPacket *avPacket = av_packet_alloc();
//    if (!avPacket) {
//        LOG("failed to av_packet_alloc");
//        return;
//    }

}

