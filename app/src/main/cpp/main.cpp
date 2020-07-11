

//#include <player/yuvplayer.h>
//#include <player/pcmplayer.h>
#include <player/lfplayer.h>

int main(int argc, char *argv[]) {
//    playYUV(argv);
//    playPCM(argv[2]);
    LOG("arg1 %s arg2 %d arg3 %d ", argv[1], atoi(argv[2]), atoi(argv[3]));
    startPlay(argv[1], atoi(argv[2]), atoi(argv[3]));
    return 0;
}

