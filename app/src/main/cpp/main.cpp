

//#include <player/yuvplayer.h>
//#include <player/pcmplayer.h>
#include <player/lfplayer.h>

int CharToInt(const char *src, long len) {
    int i;
    int value_temp = 0;

    for (i = 0; i < len; i++) {
        value_temp = value_temp * 10 + (*(src + i) - '0');
    }
    return value_temp;
}

int main(int argc, char *argv[]) {
//    playYUV(argv);
//    playPCM(argv[2]);
    LOG("arg1 %s arg2 %s arg3 %s ", argv[1], argv[2], argv[3]);
    startPlay(argv[1]);
    return 0;
}

