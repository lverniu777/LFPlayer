#include <jni.h>
#include <string>
#include <android/log.h>

extern "C" {
#include <SDL2/SDL.h>
#include <SDL2/SDL_android.h>
}

#define BLOCK_SIZE 4096000

//event message
#define REFRESH_EVENT  (SDL_USEREVENT + 1)
#define QUIT_EVENT  (SDL_USEREVENT + 2)

int thread_exit = 0;

int refresh_video_timer(void *udata) {
    thread_exit = 0;
    SDL_Event event;
    while (!thread_exit) {
        event.type = REFRESH_EVENT;
        SDL_PushEvent(&event);
        SDL_Delay(40);
    }
    thread_exit = 0;
    event.type = QUIT_EVENT;
    SDL_PushEvent(&event);
    return 0;
}

int main(int argc, char *argv[]) {
    FILE *video_fd = NULL;
    SDL_Event event;
    Uint32 pixformat = 0;
    SDL_Window *win = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    SDL_Thread *timer_thread = NULL;

    int w_width = 1080, w_height = 1920;
    const int video_width = 720, video_height = 1280;

    Uint8 *video_pos = NULL;
    Uint8 *video_end = NULL;

    unsigned int remain_len = 0;
    unsigned int video_buff_len = 0;
    unsigned int blank_space_len = 0;


    const char *path = argv[1];

    const unsigned int yuvFrameLen = video_width * video_height * 12 / 8;
    Uint8 *video_buf = (Uint8 *) (malloc(BLOCK_SIZE));
    //initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }

    //creat window from SDL
    win = SDL_CreateWindow("YUV Player",
                           SDL_WINDOWPOS_UNDEFINED,
                           SDL_WINDOWPOS_UNDEFINED,
                           w_width, w_height,
                           SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!win) {
        fprintf(stderr, "Failed to create window, %s\n", SDL_GetError());
        goto __FAIL;
    }

    renderer = SDL_CreateRenderer(win, -1, 0);

    //IYUV: Y + U + V  (3 planes)
    //YV12: Y + V + U  (3 planes)
    pixformat = SDL_PIXELFORMAT_IYUV;

    //create texture for render
    texture = SDL_CreateTexture(renderer,
                                pixformat,
                                SDL_TEXTUREACCESS_STREAMING,
                                video_width,
                                video_height);

    //open yuv file
    video_fd = fopen(path, "r");
    if (!video_fd) {
        fprintf(stderr, "Failed to open yuv file\n");
        goto __FAIL;
    }

    //read block data
    if ((video_buff_len = fread(video_buf, 1, yuvFrameLen, video_fd)) <= 0) {
        fprintf(stderr, "Failed to read data from yuv file!\n");
        goto __FAIL;
    }

    //set video positon
    video_pos = video_buf;
    video_end = video_buf + video_buff_len;
    blank_space_len = BLOCK_SIZE - video_buff_len;

    timer_thread = SDL_CreateThread(refresh_video_timer,
                                    NULL,
                                    NULL);

    do {
        //Wait
        SDL_WaitEvent(&event);
        if (event.type == REFRESH_EVENT) {
            //not enought data to render
            if ((video_pos + yuvFrameLen) > video_end) {

                //have remain data, but there isn't space
                remain_len = video_end - video_pos;
                if (remain_len && !blank_space_len) {
                    //copy data to header of buffer
                    memcpy(video_buf, video_pos, remain_len);

                    blank_space_len = BLOCK_SIZE - remain_len;
                    video_pos = video_buf;
                    video_end = video_buf + remain_len;
                }

                //at the end of buffer, so rotate to header of buffer
                if (video_end == (video_buf + BLOCK_SIZE)) {
                    video_pos = video_buf;
                    video_end = video_buf;
                    blank_space_len = BLOCK_SIZE;
                }

                //read data from yuv file to buffer
                if ((video_buff_len = fread(video_end, 1, blank_space_len, video_fd)) <= 0) {
                    fprintf(stderr, "eof, exit thread!");
                    thread_exit = 1;
                    continue;// to wait event for exiting
                }

                //reset video_end
                video_end += video_buff_len;
                blank_space_len -= video_buff_len;
                printf("not enought data: pos:%p, video_end:%p, blank_space_len:%d\n", video_pos,
                       video_end, blank_space_len);
            }
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            __android_log_print(ANDROID_LOG_ERROR, "LFPlayer", "sdl start update texture");
            SDL_UpdateTexture(texture, NULL, video_pos, video_width);
            __android_log_print(ANDROID_LOG_ERROR, "LFPlayer", "sdl update texture success ");
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);

            printf("not enought data: pos:%p, video_end:%p, blank_space_len:%d\n", video_pos,
                   video_end, blank_space_len);
            video_pos += yuvFrameLen;

        } else if (event.type == SDL_WINDOWEVENT) {
            //If Resize
            SDL_GetWindowSize(win, &w_width, &w_height);
        } else if (event.type == SDL_QUIT) {
            thread_exit = 1;
        } else if (event.type == QUIT_EVENT) {
            break;
        }
    } while (1);

    __FAIL:

    //close file
    if (video_fd) {
        fclose(video_fd);
    }


    SDL_Quit();

    return 0;
}

