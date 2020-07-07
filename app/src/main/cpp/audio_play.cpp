//
// Created by 79852 on 2020/7/4.
//

#include <player/lfplayerutils.h>
#include <jni.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_lfplayer_AudioPlayActivity_nativePlayAudio(JNIEnv
                                                            *env,
                                                            jobject thiz,
                                                            jstring
                                                            audio_file_path,
                                                            jobject audio_play_activity) {
    const char *path = env->GetStringUTFChars(audio_file_path, nullptr);
    int ret = -1;
    AVFormatContext *avFormatContext = nullptr;
    ret = avformat_open_input(&avFormatContext, path, nullptr, nullptr);
    avformat_find_stream_info(avFormatContext, nullptr);
    int audioStreamIndex = -1;
    for (int i = 0; i < avFormatContext->nb_streams; i++) {
        AVStream *stream = avFormatContext->streams[i];
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
        }
    }
    const AVStream *audioStream = avFormatContext->streams[audioStreamIndex];
    const AVCodecParameters *audioParameter = audioStream->codecpar;
    LOG("audio bitrate: %ld, codec name: %s ", audioParameter->bit_rate,
        avcodec_get_name(audioParameter->codec_id));
    AVPacket *packet = av_packet_alloc();
    av_init_packet(packet);
    AVCodec *codec = avcodec_find_decoder(audioParameter->codec_id);
    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext, audioParameter);
    avcodec_open2(codecContext, codec, nullptr);
    AVFrame *frame = av_frame_alloc();
    frame->format = audioParameter->format;
    frame->channels = audioParameter->channels;
    frame->sample_rate = audioParameter->sample_rate;
    frame->channel_layout = audioParameter->channel_layout;
    frame->nb_samples = codecContext->frame_size;
    av_frame_get_buffer(frame, 0);
    jclass jclazz = env->GetObjectClass(audio_play_activity);
    jmethodID audioPlayMethodID = env->GetMethodID(jclazz,
                                                   "audioPlay", "([BII)V");
    SwrContext *swrContext = swr_alloc();
    swr_alloc_set_opts(swrContext,
                       AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16,
                       44100,
                       frame->channel_layout, (AVSampleFormat) frame->format, frame->sample_rate,
                       0, nullptr);
    swr_init(swrContext);
    uint8_t *audioBuffer = nullptr;
    int audioBufferLineSize = 0;
    ret = av_samples_alloc(&audioBuffer, &audioBufferLineSize, 2, frame->nb_samples,
                           AV_SAMPLE_FMT_S16,
                           0);
    while (av_read_frame(avFormatContext, packet) >= 0) {
        if (packet->stream_index != audioStreamIndex) {
            continue;
        }
        ret = avcodec_send_packet(codecContext, packet);
        while ((ret = avcodec_receive_frame(codecContext, frame)) >= 0) {
            ret = swr_convert(swrContext, &audioBuffer, frame->nb_samples,
                              (const uint8_t **) frame->data, frame->nb_samples);
            jbyte *jbytes = (jbyte *) (malloc(frame->linesize[0]));
            memcpy(jbytes, audioBuffer, frame->linesize[0]);
            jbyteArray byteArray = env->NewByteArray(frame->linesize[0]);
            env->SetByteArrayRegion(byteArray, 0, frame->linesize[0], jbytes);
            env->CallVoidMethod(audio_play_activity, audioPlayMethodID, byteArray,
                                frame->sample_rate, frame->channels);
        }
        av_packet_unref(packet);
    }
}


