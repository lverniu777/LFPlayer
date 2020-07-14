//
// Created by 79852 on 2020/5/5.
//
#include <player/lfplayer.h>

extern "C" {
#include <libyuv/rotate.h>
#include <libavutil/imgutils.h>
}

void startPlay(char *path, const int windowWidth, const int windowHeight) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        LOG("SDL_Init failed: %s", SDL_GetError());
        return;
    }
    //多媒体上下文，这里一定要初始化位NULL，不然avformat_open_input会报错
    AVFormatContext *avFormatContext = nullptr;
    int ret = avformat_open_input(&avFormatContext, path, nullptr, nullptr);
    if (ret) {
        LOG("avformat open input  failed: %s", av_err2str(ret));
        return;
    }
    if (avformat_find_stream_info(avFormatContext, nullptr)) {
        LOG("avformat find stream info failed: %s", av_err2str(ret));
        return;
    }
    int videoStreamIndex = -1;
    for (int i = 0; i < avFormatContext->nb_streams; i++) {
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1) {
        LOG("no find video stream");
        return;
    }
    AVStream *videoStream = avFormatContext->streams[videoStreamIndex];
    AVDictionaryEntry *dictionaryEntry = av_dict_get(videoStream->metadata, "rotate",
                                                     nullptr, AV_DICT_MATCH_CASE);
    int rotate;
    if (dictionaryEntry) {
        rotate = atoi(dictionaryEntry->value);
    } else {
        rotate = 0;
    }
    //原始视频流的编解码上下文
    AVCodecParameters *videoCodecParameters = avFormatContext->streams[videoStreamIndex]->codecpar;
    //解码器
    AVCodec *videoCodec = avcodec_find_decoder(videoCodecParameters->codec_id);
    if (!videoCodec) {
        LOG("no find video codec: %s", videoCodec->name);
        return;
    }
    //编解码上下文
    AVCodecContext *videoCodecContext = avcodec_alloc_context3(videoCodec);
    if ((ret = avcodec_parameters_to_context(videoCodecContext, videoCodecParameters))) {
        LOG("avcodec_copy_context failed %s", av_err2str(ret));
        return;
    }
    if ((ret = avcodec_open2(videoCodecContext, videoCodec, nullptr))) {
        LOG("avcodec_open2 failed %s", av_err2str(ret));
        return;
    }
    const int videoWidth = videoCodecContext->width;
    const int videoHeight = videoCodecContext->height;
    //渲染窗口
    SDL_Window *sdlWindow = SDL_CreateWindow(TAG, 0, 0, windowWidth, windowHeight,
                                             SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    LOG("video width：%d video height: %d window width: %d window height: %d ", videoWidth,
        videoHeight, windowWidth, windowHeight);
    if (!sdlWindow) {
        LOG("SDL_CreateWindow failed %s", SDL_GetError());
        return;
    }
    //SDL渲染器
    SDL_Renderer *sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!sdlRenderer) {
        LOG("SDL_CreateRenderer failed %s", SDL_GetError());
        return;
    }
    //渲染纹理
    SDL_Texture *sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV,
                                                SDL_TEXTUREACCESS_STREAMING, videoWidth,
                                                videoHeight);
    if (!sdlTexture) {
        LOG("SDL_CreateTexture failed %s", SDL_GetError());
        return;
    }
    SwsContext *swsContext = nullptr;
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
    uint8_t *pictureData[4];
    int pictureLineSize[4];
    av_image_alloc(pictureData, pictureLineSize, videoWidth, videoHeight, AV_PIX_FMT_YUV420P, 32);
    //解码前的数据包(可以包含一个或多个视频帧)
    AVPacket *avPacket = av_packet_alloc();
    //解码后的数据帧
    AVFrame *avFrame = av_frame_alloc();
    //SDL渲染时使用的渲染区域
    SDL_Rect sdlRect;
    while (av_read_frame(avFormatContext, avPacket) >= 0) {
        if (avPacket->stream_index == videoStreamIndex) {
            //解码视频帧
            if (!avcodec_send_packet(videoCodecContext, avPacket)) {
                while (!avcodec_receive_frame(videoCodecContext, avFrame)) {
                    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
                    SDL_RenderClear(sdlRenderer);
                    sws_scale(swsContext, (uint8_t const *const *) avFrame->data,
                              avFrame->linesize, 0, videoHeight,
                              pictureData, pictureLineSize);

                    sdlRect.x = (windowWidth - videoWidth) / 2;
                    sdlRect.y = (windowHeight - videoHeight) / 2;
                    sdlRect.w = videoWidth;
                    sdlRect.h = videoHeight;
                    LOG("sdlRect h: %d", sdlRect.h);

                    SDL_UpdateYUVTexture(sdlTexture, nullptr,
                                         pictureData[0], pictureLineSize[0],
                                         pictureData[1], pictureLineSize[1],
                                         pictureData[2], pictureLineSize[2]);
                    SDL_RenderCopyEx(sdlRenderer, sdlTexture, nullptr, &sdlRect, rotate, nullptr,
                                     SDL_FLIP_NONE);
                    SDL_RenderPresent(sdlRenderer);
                }
            }
        } else {
            av_packet_unref(avPacket);
        }
    }
    av_freep(pictureData);
    av_packet_free(&avPacket);
    avformat_free_context(avFormatContext);
    SDL_DestroyTexture(sdlTexture);
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);
}
