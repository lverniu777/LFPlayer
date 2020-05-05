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
#include <SDL2/SDL.h>
}

void startPlay(char *);

#endif //LFPLAYER_LFPLAYER_H
