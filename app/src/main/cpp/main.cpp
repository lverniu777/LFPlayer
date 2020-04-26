#include <jni.h>
#include <string>
#include <android/log.h>

extern "C" {
#include <SDL2/SDL.h>
#include <SDL2/SDL_android.h>
}

int main(int argc, char *argv[]) {
    const char *TAG = "LFPlayer";
    const int width = 1080;
    const int height = 1920;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "sdl init error %s", SDL_GetError());
        return -1;
    }
    JNIEnv *jniEnv = Android_JNI_GetEnv();
    __android_log_print(ANDROID_LOG_ERROR, TAG, "jni version %d", jniEnv->GetVersion());

    SDL_Window *sdlWindow = SDL_CreateWindow("HelloWorld", 0, 0, width, height, SDL_WINDOW_SHOWN);
    if (sdlWindow == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "create window error %s", SDL_GetError());
        return -1;
    }
    SDL_Renderer *sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);
    if (sdlRenderer == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "create renderer error %s", SDL_GetError());
        return -1;
    }
    SDL_Texture *sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ABGR8888,
                                                SDL_TEXTUREACCESS_TARGET, width, height);
    if (sdlTexture == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "create texture error %s", SDL_GetError());
        return -1;
    }
    SDL_Rect sdlRect;
    sdlRect.w = 200;
    sdlRect.h = 200;
    int exit = 0;
    SDL_Event sdlEvent;
    while (!exit) {
        if (SDL_PollEvent(&sdlEvent)) {
            if (sdlEvent.type == SDL_QUIT) {
                exit = 1;
            }
            __android_log_print(ANDROID_LOG_ERROR, TAG, "sdl event %d", sdlEvent.type);
        }
        SDL_SetRenderTarget(sdlRenderer, sdlTexture);
        SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 255);
        SDL_RenderClear(sdlRenderer);
        sdlRect.x = rand() % (width - sdlRect.w);
        sdlRect.y = rand() % (height - sdlRect.h);
        SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(sdlRenderer, &sdlRect);
        SDL_RenderFillRect(sdlRenderer, &sdlRect);
        SDL_SetRenderTarget(sdlRenderer, NULL);
        SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
        SDL_RenderPresent(sdlRenderer);
    }

    SDL_DestroyTexture(sdlTexture);
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();
    return 0;
}
