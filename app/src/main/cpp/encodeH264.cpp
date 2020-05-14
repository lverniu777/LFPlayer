//
// Created by 79852 on 2020/5/14.
//
#include <jni.h>
#include <player/lfplayerutils.h>

extern "C"
JNIEXPORT void JNICALL
Java_com_example_lfplayer_encoder_H264Encoder_encodeH264(JNIEnv *env, jobject thiz,
                                                         jbyteArray frame, jint width, jint height,
                                                         jstring file_path) {
    LOG(env->GetStringUTFChars(file_path, NULL));
}
