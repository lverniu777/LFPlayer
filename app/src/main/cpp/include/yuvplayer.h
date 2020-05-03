//
// Created by 79852 on 2020/5/2.
//

#ifndef LFPLAYER_YUVPLAYER_H
#define LFPLAYER_YUVPLAYER_H
#define BLOCK_SIZE 4096000
extern "C" {
#include <SDL2/SDL.h>
#include <SDL2/SDL_android.h>
}
//event message
#define REFRESH_EVENT  (SDL_USEREVENT + 1)
#define QUIT_EVENT  (SDL_USEREVENT + 2)

int refresh_video_timer(void *);

void playYUV(char **);

#endif //LFPLAYER_YUVPLAYER_H
