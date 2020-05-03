//
// Created by 79852 on 2020/5/3.
//
#include "player/pcmplayer.h"

/**
 * 音频文件缓冲区
 */
Uint8 *audioBuf = NULL;
/**
 * 当前文件缓存区的位置
 */
static Uint8 *audioPos = NULL;

/**
 * 缓冲区剩余大小
 */
static size_t leftLen = 0;

void readAudioCallback(void *userdata, Uint8 *stream, int len) {
    if (leftLen <= 0) {
        return;
    }
    SDL_memset(stream, 0, len);
    len = len < leftLen ? len : leftLen;
    SDL_MixAudio(stream, audioPos, len, SDL_MIX_MAXVOLUME);
    LOG("read from buffer: %d len: %d",len,leftLen);
    audioPos += len;
    leftLen -= len;
}

void playPCM(char *playPath) {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
        LOG("SDL init error")
        return;
    }
    SDL_AudioSpec sdlAudioSpec;
    sdlAudioSpec.format = AUDIO_S16SYS;
    sdlAudioSpec.channels = 2;
    //指定SDL音频读取的回调函数
    sdlAudioSpec.callback = readAudioCallback;
    sdlAudioSpec.freq = 44100;
    sdlAudioSpec.userdata = NULL;
    if (SDL_OpenAudio(&sdlAudioSpec, NULL)) {
        LOG("open audio error");
        return;
    }
    FILE *pcmFile = fopen(playPath, "r");
    if (!pcmFile) {
        LOG("fopen error : %s", playPath);
        return;
    }
    SDL_PauseAudio(0);
    audioBuf = (Uint8 *) (malloc(BLOCK_SIZE));
    size_t readLen = 0;
    do {
        readLen = fread(audioBuf, 1, BLOCK_SIZE, pcmFile);
        leftLen = readLen;
        audioPos = audioBuf;
        LOG("read from pcm file readLen: %d leftLen: %d",readLen,leftLen);
        while (audioPos < audioBuf + readLen) {
            SDL_Delay(1);
        }
    } while (readLen != 0);
    free(audioBuf);
    fclose(pcmFile);
    SDL_CloseAudio();
    SDL_Quit();
}

