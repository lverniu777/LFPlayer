//
// Created by 79852 on 2020/5/5.
//
#include <player/lfplayer.h>

static PacketQueue audioq;
static SwrContext *audioConvertCtx = NULL;
static int64_t inChannelLayout;
static int64_t outChannelLayout;
int8_t quit = 0;

int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block) {
    AVPacketList *pkt1;
    int ret;

    SDL_LockMutex(q->mutex);

    for (;;) {

        if (quit) {
            ret = -1;
            break;
        }

        pkt1 = q->first_pkt;
        if (pkt1) {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt)
                q->last_pkt = NULL;
            q->nb_packets--;
            q->size -= pkt1->pkt.size;
            *pkt = pkt1->pkt;
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {

    static AVPacket pkt;
    static uint8_t *audio_pkt_data = NULL;
    static int audio_pkt_size = 0;
    static AVFrame frame;

    int len1, data_size = 0;

    for (;;) {
        while (audio_pkt_size > 0) {
            int got_frame = 0;
            len1 = avcodec_decode_audio4(aCodecCtx, &frame, &got_frame, &pkt);
            if (len1 < 0) {
                /* if error, skip frame */
                audio_pkt_size = 0;
                break;
            }
            audio_pkt_data += len1;
            audio_pkt_size -= len1;
            data_size = 0;
            if (got_frame) {
                //fprintf(stderr, "channels:%d, nb_samples:%d, sample_fmt:%d \n", aCodecCtx->channels, frame.nb_samples, aCodecCtx->sample_fmt);
                /*
            data_size = av_samples_get_buffer_size(NULL,
                                   aCodecCtx->channels,
                                   frame.nb_samples,
                                   aCodecCtx->sample_fmt,
                                   1);
                */
                data_size = 2 * 2 * frame.nb_samples;
                swr_convert(audioConvertCtx,
                            &audio_buf,
                            MAX_AUDIO_FRAME_SIZE * 3 / 2,
                            (const uint8_t **) frame.data,
                            frame.nb_samples);

                //memcpy(audio_buf, frame.data[0], data_size);
            }
            if (data_size <= 0) {
                /* No data yet, get more frames */
                continue;
            }
            /* We have data, return it and come back for more later */
            return data_size;
        }
        if (pkt.data)
            av_free_packet(&pkt);

        if (quit) {
            return -1;
        }

        if (packet_queue_get(&audioq, &pkt, 1) < 0) {
            return -1;
        }
        audio_pkt_data = pkt.data;
        audio_pkt_size = pkt.size;
    }
}

void audio_callback(void *userdata, Uint8 *stream, int len) {
    AVCodecContext *aCodecCtx = (AVCodecContext *) userdata;
    int len1, audio_size;

    static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;

    while (len > 0) {
        if (audio_buf_index >= audio_buf_size) {
            audio_size = audio_decode_frame(aCodecCtx, audio_buf, sizeof(audio_buf));
            if (audio_size < 0) {
                /* If error, output silence */
                audio_buf_size = 1024; // arbitrary?
                memset(audio_buf, 0, audio_buf_size);
            } else {
                audio_buf_size = audio_size;
            }
            audio_buf_index = 0;
        }
        len1 = audio_buf_size - audio_buf_index;
        if (len1 > len)
            len1 = len;
        memcpy(stream, (uint8_t *) audio_buf + audio_buf_index, len1);
        len -= len1;
        stream += len1;
        audio_buf_index += len1;
    }
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
    AVPacketList *pkt1;
    if (av_dup_packet(pkt) < 0) {
        return -1;
    }
    pkt1 = (AVPacketList *) (av_malloc(sizeof(AVPacketList)));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;

    SDL_LockMutex(q->mutex);

    if (!q->last_pkt) {
        q->first_pkt = pkt1;
    } else {
        q->last_pkt->next = pkt1;
    }

    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size;
    SDL_CondSignal(q->cond);

    SDL_UnlockMutex(q->mutex);
    return 0;
}


void packet_queue_init(PacketQueue *q) {
    memset(q, 0, sizeof(PacketQueue));
    q->mutex = SDL_CreateMutex();
    q->cond = SDL_CreateCond();
}

void startPlay(char *path) {
    LOG("file path: %s", path);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        LOG("SDL_Init failed: %s", SDL_GetError());
        return;
    }
    //多媒体上下文，这里一定要初始化位NULL，不然avformat_open_input会报错
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
    int audioStreamIndex = -1;
    for (int i = 0; i < avFormatContext->nb_streams; i++) {
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
        }
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
        }
    }
    if (videoStreamIndex == -1) {
        LOG("no find video stream");
        return;
    }
    if (audioStreamIndex == -1) {
        LOG("no find audio stream");
        return;
    }
    AVCodecContext *originAudioCodecContext = avFormatContext->streams[audioStreamIndex]->codec;
    AVCodec *audioCodec = avcodec_find_decoder(originAudioCodecContext->codec_id);
    if (!audioCodec) {
        LOG("no find audio codec: %s", audioCodec->name);
        return;
    }
    AVCodecContext *audioCodecContext = avcodec_alloc_context3(audioCodec);
    if ((ret = avcodec_copy_context(audioCodecContext, originAudioCodecContext))) {
        LOG("audio avcodec_copy_context failed %s", av_err2str(ret));
        return;
    }
    SDL_AudioSpec desiredAudioSpec, obtainedAudioSpec;
    desiredAudioSpec.freq = audioCodecContext->sample_rate;
    desiredAudioSpec.channels = audioCodecContext->channels;
    desiredAudioSpec.format = AUDIO_S16SYS;
    desiredAudioSpec.silence = 0;
    desiredAudioSpec.samples = SDL_AUDIO_BUFFER_SIZE;
    desiredAudioSpec.userdata = audioCodecContext;
    desiredAudioSpec.callback = audio_callback;
    if ((SDL_OpenAudio(&desiredAudioSpec, &obtainedAudioSpec))) {
        LOG("SDL_OpenAudio failed %s", SDL_GetError());
        return;
    }
    if ((ret = avcodec_open2(audioCodecContext, NULL, NULL))) {
        LOG("audio avcodec_open2 failed %s", av_err2str(ret));
        return;
    }
    packet_queue_init(&audioq);
    inChannelLayout = av_get_default_channel_layout(audioCodecContext->channels);
    outChannelLayout = AV_CH_LAYOUT_STEREO;
    audioConvertCtx = swr_alloc();
    if (!audioConvertCtx) {
        LOG("swr_alloc failed");
        return;
    }
    swr_alloc_set_opts(audioConvertCtx,
                       outChannelLayout,
                       AV_SAMPLE_FMT_S16,
                       audioCodecContext->sample_rate,
                       inChannelLayout,
                       audioCodecContext->sample_fmt,
                       audioCodecContext->sample_rate,
                       0,
                       NULL);
    swr_init(audioConvertCtx);
    SDL_PauseAudio(0);
    //原始视频流的编解码上下文
    AVCodecContext *originVideoCodecContext = avFormatContext->streams[videoStreamIndex]->codec;
    //解码器
    AVCodec *videoCodec = avcodec_find_decoder(originVideoCodecContext->codec_id);
    if (!videoCodec) {
        LOG("no find video codec: %s", videoCodec->name);
        return;
    }

    //编解码上下文
    AVCodecContext *videoCodecContext = avcodec_alloc_context3(videoCodec);
    if ((ret = avcodec_copy_context(videoCodecContext, originVideoCodecContext))) {
        LOG("avcodec_copy_context failed %s", av_err2str(ret));
        return;
    }
    if ((ret = avcodec_open2(videoCodecContext, videoCodec, NULL))) {
        LOG("avcodec_open2 failed %s", av_err2str(ret));
        return;
    }
    const int videoWidth = videoCodecContext->width;
    const int videoHeight = videoCodecContext->height;
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
                                            videoCodecContext->pix_fmt,
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
            avcodec_decode_video2(videoCodecContext, avFrame, &ret, &avPacket);
            if (ret) {
                SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
                SDL_RenderClear(sdlRenderer);
                // Set Size of Window
                sdlRect.x = 0;
                sdlRect.y = 0;
                sdlRect.w = videoWidth;
                sdlRect.h = videoHeight;
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
        } else if (avPacket.stream_index == audioStreamIndex) {
            packet_queue_put(&audioq, &avPacket);
        } else {
            av_packet_unref(&avPacket);
        }
    }
}
