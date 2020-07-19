//
// Created by 79852 on 2020/5/5.
//
#include <player/lfplayer.h>
#include <queue>
#include <jni.h>

extern "C" {
#include <libyuv/rotate.h>
#include <libavutil/imgutils.h>
}
typedef struct {
    uint8_t *videoData[4];
    int videoLineSize[4];
} VideoBuffer;

typedef enum {
    RESUME = 1,
    PAUSE = 2,
    FINISH = 3
} State;

//多媒体上下文，这里一定要初始化位NULL，不然avformat_open_input会报错
AVFormatContext *avFormatContext = nullptr;
AVCodecContext *videoCodecContext = nullptr;
int videoStreamIndex = -1;
SwsContext *swsContext = nullptr;

SDL_mutex *videoBufferMutex;
SDL_cond *videoBufferCond;
std::queue<VideoBuffer> videoBufferQueue;
int playState = PAUSE;

SDL_Texture *sdlTexture;
SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;

static AVPacket *avPacket;

int rotation;

int decode(void *data) {
    const char *path = (char *) (data);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        LOG("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }

    int ret = avformat_open_input(&avFormatContext, path, nullptr, nullptr);
    if (ret) {
        LOG("avformat open input  failed: %s", av_err2str(ret));
        return -1;
    }
    if (avformat_find_stream_info(avFormatContext, nullptr)) {
        LOG("avformat find stream info failed: %s", av_err2str(ret));
        return -1;
    }
    videoStreamIndex = -1;
    for (int i = 0; i < avFormatContext->nb_streams; i++) {
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1) {
        LOG("no find video stream");
        return -1;
    }
    AVStream *videoStream = avFormatContext->streams[videoStreamIndex];
    AVDictionaryEntry *dictionaryEntry = av_dict_get(videoStream->metadata, "rotate",
                                                     nullptr, AV_DICT_MATCH_CASE);
    if (dictionaryEntry) {
        rotation = atoi(dictionaryEntry->value);
    } else {
        rotation = 0;
    }
    //原始视频流的编解码上下文
    AVCodecParameters *videoCodecParameters = avFormatContext->streams[videoStreamIndex]->codecpar;
    //解码器
    AVCodec *videoCodec = avcodec_find_decoder(videoCodecParameters->codec_id);
    if (!videoCodec) {
        LOG("no find video codec: %s", videoCodec->name);
        return -1;
    }
    videoCodecContext = avcodec_alloc_context3(videoCodec);
    if ((ret = avcodec_parameters_to_context(videoCodecContext, videoCodecParameters))) {
        LOG("avcodec_copy_context failed %s", av_err2str(ret));
        return -1;
    }
    if ((ret = avcodec_open2(videoCodecContext, videoCodec, nullptr))) {
        LOG("avcodec_open2 failed %s", av_err2str(ret));
        return -1;
    }

    const int videoWidth = videoCodecContext->width;
    const int videoHeight = videoCodecContext->height;
    swsContext = sws_getContext(videoWidth,
                                videoHeight,
                                videoCodecContext->pix_fmt,
                                videoWidth,
                                videoHeight,
                                AV_PIX_FMT_YUV420P,
                                SWS_BILINEAR,
                                nullptr,
                                nullptr,
                                nullptr
    );
    //解码前的数据包(可以包含一个或多个视频帧)
    avPacket = av_packet_alloc();
    //解码后的数据帧
    AVFrame *avFrame = av_frame_alloc();
    while (avFormatContext && av_read_frame(avFormatContext, avPacket) >= 0) {
        if (avPacket->stream_index == videoStreamIndex) {
            //解码视频帧
            if (!avcodec_send_packet(videoCodecContext, avPacket)) {
                while (!avcodec_receive_frame(videoCodecContext, avFrame)) {
                    uint8_t *videoData[4];
                    int videoLineSize[4];
                    av_image_alloc(videoData, videoLineSize, videoCodecContext->width,
                                   videoCodecContext->height, AV_PIX_FMT_YUV420P, 32);
                    sws_scale(swsContext, (uint8_t const *const *) avFrame->data,
                              avFrame->linesize, 0, videoHeight,
                              videoData, videoLineSize);
                    SDL_LockMutex(videoBufferMutex);
                    VideoBuffer videoBuffer = VideoBuffer();
                    for (int i = 0; i < 4; i++) {
                        videoBuffer.videoData[i] = videoData[i];
                        videoBuffer.videoLineSize[i] = videoLineSize[i];
                    }
                    videoBufferQueue.push(videoBuffer);
                    SDL_CondBroadcast(videoBufferCond);
                    SDL_UnlockMutex(videoBufferMutex);
                }
            }
        } else {
            av_packet_unref(avPacket);
        }
    }
    return 1;
}

int render(void *data) {
    SDL_LockMutex(videoBufferMutex);
    while (!videoCodecContext) {
        SDL_CondWait(videoBufferCond, videoBufferMutex);
    }
    SDL_UnlockMutex(videoBufferMutex);
    const int videoWidth = videoCodecContext->width;
    const int videoHeight = videoCodecContext->height;
    //渲染窗口
    sdlWindow = SDL_CreateWindow(
            TAG, 0, 0,
            0, 0,
            SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN
    );
    int w, h;
    SDL_GetWindowSize(sdlWindow, &w, &h);
    if (!sdlWindow) {
        LOG("SDL_CreateWindow failed %s", SDL_GetError());
        return -1;
    }
    //SDL渲染器
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!sdlRenderer) {
        LOG("SDL_CreateRenderer failed %s", SDL_GetError());
        return -1;
    }
    //渲染纹理
    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   videoCodecContext->width,
                                   videoCodecContext->height);
    if (!sdlTexture) {
        LOG("SDL_CreateTexture failed %s", SDL_GetError());
        return -1;
    }
    //SDL渲染时使用的渲染区域
    SDL_Rect sdlRect;
    sdlRect.x = (w - videoWidth) / 2;
    sdlRect.y = (h - videoHeight) / 2;
    sdlRect.w = videoWidth;
    sdlRect.h = videoHeight;
    while (true) {
        SDL_LockMutex(videoBufferMutex);
        if (playState == FINISH) {
            SDL_DestroyTexture(sdlTexture);
            SDL_DestroyRenderer(sdlRenderer);
            SDL_DestroyWindow(sdlWindow);
            SDL_UnlockMutex(videoBufferMutex);
            break;
        }
        while (videoBufferQueue.empty() || playState == PAUSE) {
            SDL_CondWait(videoBufferCond, videoBufferMutex);
        }
        VideoBuffer videoBuffer = videoBufferQueue.front();
        videoBufferQueue.pop();
        SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
        SDL_RenderClear(sdlRenderer);
        SDL_UpdateYUVTexture(sdlTexture, nullptr,
                             videoBuffer.videoData[0], videoBuffer.videoLineSize[0],
                             videoBuffer.videoData[1], videoBuffer.videoLineSize[1],
                             videoBuffer.videoData[2], videoBuffer.videoLineSize[2]);
        SDL_RenderCopyEx(sdlRenderer, sdlTexture, nullptr, &sdlRect, rotation, nullptr,
                         SDL_FLIP_NONE);
        SDL_RenderPresent(sdlRenderer);
        SDL_UnlockMutex(videoBufferMutex);
    }
    return 1;
}

/**
 * SDL主线程循环接收各种SDL事件
 * @param path
 */
void startPlay(char *path) {
    playState = PAUSE;
    videoBufferMutex = SDL_CreateMutex();
    videoBufferCond = SDL_CreateCond();
    SDL_Thread *decodeThread = SDL_CreateThread(decode, "decode thread", path);
    SDL_Thread *renderThread = SDL_CreateThread(render, "render thread", nullptr);
    while (true) {
        SDL_Event sdlEvent;
        SDL_WaitEvent(&sdlEvent);
        LOG("sdl event: %d", sdlEvent.type);
        switch (sdlEvent.type) {
            case RESUME:
                SDL_LockMutex(videoBufferMutex);
                playState = RESUME;
                SDL_CondBroadcast(videoBufferCond);
                SDL_UnlockMutex(videoBufferMutex);
                break;
            case PAUSE:
                SDL_LockMutex(videoBufferMutex);
                playState = PAUSE;
                SDL_CondBroadcast(videoBufferCond);
                SDL_UnlockMutex(videoBufferMutex);
                break;
                //FIXME 暂时使用KEYUP事件代替返回按键
            case SDL_KEYUP:
            case FINISH:
                playState = FINISH;
                goto exit;
        }
    }
    exit:
    SDL_DetachThread(decodeThread);
    SDL_DetachThread(renderThread);
    av_packet_free(&avPacket);
    avformat_close_input(&avFormatContext);
    SDL_DestroyTexture(sdlTexture);
    sdlTexture = nullptr;
    SDL_DestroyRenderer(sdlRenderer);
    sdlRenderer = nullptr;
    SDL_DestroyWindow(sdlWindow);
    sdlWindow = nullptr;
    SDL_DestroyCond(videoBufferCond);
    videoBufferCond = nullptr;
    SDL_DestroyMutex(videoBufferMutex);
    videoBufferMutex = nullptr;
}

extern "C"
JNIEXPORT void JNICALL
Java_org_libsdl_app_SDLActivity_nativePlayerEvent(JNIEnv *env, jobject thiz, jint event_type) {
    SDL_Event playerEvent;
    switch (event_type) {
        case RESUME:
            playerEvent.type = RESUME;
            break;
        case PAUSE:
            playerEvent.type = PAUSE;
            break;
        case FINISH:
            playerEvent.type = FINISH;
            break;
    }
    SDL_PushEvent(&playerEvent);
}
