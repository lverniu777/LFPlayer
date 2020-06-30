//
// Created by 79852 on 2020/6/30.
//
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}
extern "C"
JNIEXPORT void JNICALL
Java_surfaceplayer_SurfaceCallback_nativePlay(JNIEnv *env, jobject thiz, jstring play_path,
                                              jobject surface) {
    const char *playPath = env->GetStringUTFChars(play_path, nullptr);
    const int displayWidth = 1080;
    const int displayHeight = 1920;
    AVFormatContext *avFormatContext = avformat_alloc_context();
    int ret = avformat_open_input(&avFormatContext, playPath, nullptr, nullptr);
    ret = avformat_find_stream_info(avFormatContext, nullptr);
    int videoStreamIndex = -1;
    for (int i = 0; i < avFormatContext->nb_streams; i++) {
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    AVStream *videoStream = avFormatContext->streams[videoStreamIndex];
    AVCodec *videoDecoder = avcodec_find_decoder(videoStream->codecpar->codec_id);
    AVCodecContext *avCodecContext = avcodec_alloc_context3(videoDecoder);
    ret = avcodec_parameters_to_context(avCodecContext, videoStream->codecpar);
    AVPacket *avPacket = av_packet_alloc();
    av_init_packet(avPacket);
    AVFrame *readFrame = av_frame_alloc();
    readFrame->width = videoStream->codecpar->width;
    readFrame->height = videoStream->codecpar->height;
    readFrame->format = videoStream->codecpar->format;
    ret = av_frame_get_buffer(readFrame, 32);
    AVFrame *renderFrame = av_frame_alloc();
    renderFrame->width = displayWidth;
    renderFrame->height = displayHeight;
    renderFrame->format = AV_PIX_FMT_RGBA;
    ret = av_frame_get_buffer(renderFrame, 32);
    SwsContext *swsContext = sws_getContext(
            videoStream->codecpar->width,
            videoStream->codecpar->height,
            (AVPixelFormat) videoStream->codecpar->format,
            1080,
            1920,
            AV_PIX_FMT_RGBA,
            SWS_BILINEAR,
            nullptr,
            nullptr,
            nullptr
    );
    ANativeWindow *aNativeWindow = ANativeWindow_fromSurface(env, surface);
    ANativeWindow_setBuffersGeometry(aNativeWindow, displayWidth, displayHeight,
                                     WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer aNativeWindowBuffer;
    while (av_read_frame(avFormatContext, avPacket) >= 0) {
        if (avPacket->stream_index != videoStreamIndex) {
            continue;
        }
        while (avcodec_send_packet(avCodecContext, avPacket)) {
            while (avcodec_receive_frame(avCodecContext, readFrame) >= 0) {
                ret = sws_scale(swsContext, readFrame->data,
                                readFrame->linesize, 0,
                                readFrame->height, renderFrame->data,
                                readFrame->linesize);
                ANativeWindow_lock(aNativeWindow, &aNativeWindowBuffer, nullptr);
                uint8_t *dst = (uint8_t *) aNativeWindowBuffer.bits;
                uint8_t *src = (renderFrame->data[0]);
                // 由于window的stride和帧的stride不同,因此需要逐行复制
                for (int i = 0; i < displayHeight; i++) {
                    memcpy(dst + i * aNativeWindowBuffer.stride * 4,
                           src + i * renderFrame->linesize[0],
                           renderFrame->linesize[0]);
                }
                ANativeWindow_unlockAndPost(aNativeWindow);
            }
            av_packet_unref(avPacket);
        }

    }
    ANativeWindow_release(aNativeWindow);
    av_packet_free(&avPacket);
    av_frame_free(&readFrame);
    av_frame_free(&renderFrame);
    avcodec_close(avCodecContext);
    avcodec_free_context(&avCodecContext);
    avformat_close_input(&avFormatContext);
    avformat_free_context(avFormatContext);
    env->ReleaseStringUTFChars(play_path, playPath);
}