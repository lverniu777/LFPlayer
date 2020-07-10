//
// Created by 79852 on 2020/7/8.
//
#include <jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>

extern "C" {
#include <libavformat/avformat.h>
#include <player/lfplayerutils.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lfplayer_SurfacePlayerActivity_nativePlayVideo(JNIEnv *env, jobject thiz,
                                                                jstring to_string,
                                                                jobject surface) {
    const char *filePath = env->GetStringUTFChars(to_string, nullptr);
    int ret;
    AVFormatContext *formatContext = nullptr;
    ret = avformat_open_input(&formatContext, filePath, nullptr, nullptr);
    ret = avformat_find_stream_info(formatContext, nullptr);
    int videoStreamIndex = -1;
    for (int i = 0; i < formatContext->nb_streams; i++) {
        AVStream *stream = formatContext->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
        }
    }
    AVStream *videoStream = formatContext->streams[videoStreamIndex];
    AVCodecParameters *videoParam = videoStream->codecpar;
    AVCodec *videoCodec = avcodec_find_decoder(videoParam->codec_id);
    AVCodecContext *codecContext = avcodec_alloc_context3(videoCodec);
    avcodec_parameters_to_context(codecContext, videoParam);
    ret = avcodec_open2(codecContext, videoCodec, nullptr);
    AVFrame *avFrame = av_frame_alloc();
    avFrame->width = videoParam->width;
    avFrame->height = videoParam->height;
    avFrame->format = videoParam->format;
    ret = av_frame_get_buffer(avFrame, 32);
    AVPacket *packet = av_packet_alloc();
    av_init_packet(packet);
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    const int windowWidth = ANativeWindow_getWidth(nativeWindow);
    const int windowHeight = ANativeWindow_getHeight(nativeWindow);
    ret = ANativeWindow_setBuffersGeometry(
            nativeWindow,
            windowWidth,
            windowHeight,
            WINDOW_FORMAT_RGBA_8888
    );
    SwsContext *swsContext = sws_getContext(
            avFrame->width, avFrame->height, (AVPixelFormat) avFrame->format,
            windowWidth, windowHeight, (AVPixelFormat) AV_PIX_FMT_RGBA,
            SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    LOG("frame width:%d frame height:%d window width:%d window height:%d", avFrame->width,
        avFrame->height, windowWidth, windowHeight);
    uint8_t *dstData[4];
    int dstLineSize[4];
    ret = av_image_alloc(dstData, dstLineSize, windowWidth, windowHeight, AV_PIX_FMT_RGBA, 32);
    ANativeWindow_Buffer nativeWindowBuffer;
    while (av_read_frame(formatContext, packet) >= 0) {
        if (packet->stream_index != videoStreamIndex) {
            continue;
        }
        if (avcodec_send_packet(codecContext, packet) >= 0) {
            while (avcodec_receive_frame(codecContext, avFrame) >= 0) {
                ret = sws_scale(swsContext, avFrame->data,
                                avFrame->linesize, 0, avFrame->height, dstData,
                                dstLineSize);
                ANativeWindow_lock(nativeWindow, &nativeWindowBuffer, nullptr);
                LOG("window buffer width:%d window buffer height:%d", nativeWindowBuffer.width,
                    nativeWindowBuffer.height);
                for (int i = 0; i < windowHeight; i++) {
                    memcpy((uint8_t *) nativeWindowBuffer.bits + nativeWindowBuffer.stride * 4 * i,
                           dstData[0] + dstLineSize[0] * i,
                           dstLineSize[0]);
                }
                ANativeWindow_unlockAndPost(nativeWindow);
            }
        }
    }

    sws_freeContext(swsContext);
    ANativeWindow_release(nativeWindow);
    av_frame_free(&avFrame);
    avformat_close_input(&formatContext);
}