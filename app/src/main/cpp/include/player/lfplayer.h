//
// Created by 79852 on 2020/5/5.
//

#ifndef LFPLAYER_LFPLAYER_H
#define LFPLAYER_LFPLAYER_H

#include <android/log.h>

#define TAG "LFPlayer"
#define LOG(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__);
extern "C" {
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <SDL2/SDL.h>
}
#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000
typedef struct PacketQueue {
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

void startPlay(char *);


#endif //LFPLAYER_LFPLAYER_H
