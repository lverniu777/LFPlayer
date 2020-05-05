//
// Created by 79852 on 2020/5/5.
//
#include <player/lfplayer.h>

void startPlay(char *path) {
    LOG("file path: %s", path);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        LOG("SDL_Init failed: %s", SDL_GetError());
        return;
    }
    //多媒体上下文
    AVFormatContext *avFormatContext = NULL;
    int ret = avformat_open_input(&avFormatContext, path, NULL, NULL);
    if (ret) {
        LOG("avformat open input  failed: %s", av_err2str(ret));
        return;
    }
    if (avformat_find_stream_info(avFormatContext, NULL)) {
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
        LOG("no find video stream", av_err2str(ret));
        return;
    }
    //原始视频流的编解码上下文
    AVCodecContext *originCodecContext = avFormatContext->streams[videoStreamIndex]->codec;
    //解码器
    AVCodec *avCodec = avcodec_find_decoder(originCodecContext->codec_id);
    if (!avCodec) {
        LOG("no find av codec ");
        return;
    }
    //编解码上下文
    AVCodecContext *avCodecContext = avcodec_alloc_context3(avCodec);
    if ((ret = avcodec_copy_context(avCodecContext, originCodecContext))) {
        LOG("avcodec_copy_context failed %s", av_err2str(ret));
        return;
    }
    if ((ret = avcodec_open2(avCodecContext, avCodec, NULL))) {
        LOG("avcodec_open2 failed %s", av_err2str(ret));
        return;
    }
    const int videoWidth = avCodecContext->width;
    const int videoHeight = avCodecContext->height;
    const int windowWidth = 1080;
    const int windowHeight = 1920;
    //渲染窗口
    SDL_Window *sdlWindow = SDL_CreateWindow(TAG, 0, 0, windowWidth, windowHeight,
                                             SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!sdlWindow) {
        LOG("SDL_CreateWindow failed %s", SDL_GetError());
        return;
    }
    //SDL渲染器
    SDL_Renderer *sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);
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
    //图像裁剪上下文
    SwsContext *swsContext = sws_getContext(videoWidth,
                                            videoHeight,
                                            avCodecContext->pix_fmt,
                                            videoWidth,
                                            videoHeight,
                                            AV_PIX_FMT_YUV420P,
                                            SWS_BILINEAR,
                                            NULL,
                                            NULL,
                                            NULL
    );
    //存放原始图像数据
    AVPicture *avPicture = (AVPicture *) (malloc(sizeof(AVPicture)));
    avpicture_alloc(avPicture,
                    AV_PIX_FMT_YUV420P,
                    videoWidth,
                    videoHeight);
    //解码前的数据包(可以包含一个或多个视频帧)
    AVPacket avPacket;
    av_init_packet(&avPacket);
    //解码后的数据帧
    AVFrame *avFrame = av_frame_alloc();
    //SDL渲染时使用的渲染区域
    SDL_Rect sdlRect;
    while (av_read_frame(avFormatContext, &avPacket) >= 0) {
        if (avPacket.stream_index == videoStreamIndex) {
            //解码视频帧
            avcodec_decode_video2(avCodecContext, avFrame, &ret, &avPacket);
            if (ret) {
                SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
                SDL_RenderClear(sdlRenderer);
                // Set Size of Window
                sdlRect.x = 0;
                sdlRect.y = 0;
                sdlRect.w = windowWidth;
                sdlRect.h = windowHeight;
                sws_scale(swsContext, (uint8_t const *const *) avFrame->data,
                          avFrame->linesize, 0, videoHeight,
                          avPicture->data, avPicture->linesize);

                SDL_UpdateYUVTexture(sdlTexture, NULL,
                                     avPicture->data[0], avPicture->linesize[0],
                                     avPicture->data[1], avPicture->linesize[1],
                                     avPicture->data[2], avPicture->linesize[2]);
                SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
                SDL_RenderPresent(sdlRenderer);
            }
        }
    }
}
