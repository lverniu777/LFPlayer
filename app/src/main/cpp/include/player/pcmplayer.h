//
// Created by 79852 on 2020/5/3.
//

#ifndef LFPLAYER_PCMPLAYER_H
#define LFPLAYER_PCMPLAYER_H

#include <android/log.h>

#define BLOCK_SIZE 4096000
#define TAG "PCM_Player"
#define LOG(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__);
extern "C" {
#include <SDL2/SDL.h>
#include <SDL2/SDL_android.h>
}

void playPCM(char *);

void readAudioCallback(void *, Uint8 *, int);

#endif //LFPLAYER_PCMPLAYER_H
