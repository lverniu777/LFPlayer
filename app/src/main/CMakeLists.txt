cmake_minimum_required(VERSION 3.4.1)
add_definitions(-Wno-format-security)
set(libpath ${PROJECT_SOURCE_DIR}/jniLibs/${CMAKE_ANDROID_ARCH_ABI})
set(includepath cpp/include)
include_directories(
        ${includepath}
)
add_library(
        lfplayer
        SHARED
        cpp/lfplayer.cpp cpp/main.cpp cpp/yuvplayer.cpp cpp/pcmplayer.cpp cpp/encodeH264.cpp)
add_library(
        hidapi
        SHARED
        IMPORTED
)
set_target_properties(
        hidapi
        PROPERTIES IMPORTED_LOCATION
        ${libpath}/libhidapi.so
)
add_library(
        SDL2
        SHARED
        IMPORTED
)
set_target_properties(
        SDL2
        PROPERTIES IMPORTED_LOCATION
        ${libpath}/libSDL2.so
)


add_library(
        SDL_image
        SHARED
        IMPORTED
)
set_target_properties(
        SDL_image
        PROPERTIES IMPORTED_LOCATION
        ${libpath}/libSDL2_image.so
)
find_library(
        log-lib
        log)
add_library(
        avcodec
        SHARED
        IMPORTED
)
set_target_properties(
        avcodec
        PROPERTIES IMPORTED_LOCATION
        ${libpath}/libavcodec.so
)

add_library(
        avdevice
        SHARED
        IMPORTED
)
set_target_properties(
        avdevice
        PROPERTIES IMPORTED_LOCATION
        ${libpath}/libavdevice.so
)

add_library(
        avfilter
        SHARED
        IMPORTED
)
set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        ${libpath}/libavfilter.so
)

add_library(
        avformat
        SHARED
        IMPORTED
)
set_target_properties(
        avformat
        PROPERTIES IMPORTED_LOCATION
        ${libpath}/libavformat.so
)

add_library(
        avutil
        SHARED
        IMPORTED
)
set_target_properties(
        avutil
        PROPERTIES IMPORTED_LOCATION
        ${libpath}/libavutil.so
)


add_library(
        swresample
        SHARED
        IMPORTED
)
set_target_properties(
        swresample
        PROPERTIES IMPORTED_LOCATION
        ${libpath}/libswresample.so
)


add_library(
        swscale
        SHARED
        IMPORTED
)
set_target_properties(
        swscale
        PROPERTIES IMPORTED_LOCATION
        ${libpath}/libswscale.so
)

add_library(
        x264
        SHARED
        IMPORTED
)
set_target_properties(
        x264
        PROPERTIES IMPORTED_LOCATION
        ${libpath}/libx264.so
)
add_library(
        fdkaac
        SHARED
        IMPORTED
)
set_target_properties(
        fdkaac
        PROPERTIES IMPORTED_LOCATION
        ${libpath}/libfdk-aac.so
)

add_library(
        rtmp
        SHARED
        IMPORTED
)
set_target_properties(
        rtmp
        PROPERTIES IMPORTED_LOCATION
        ${libpath}/librtmp.so
)
target_link_libraries(
        lfplayer
        hidapi
        SDL2
        SDL_image
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        swresample
        swscale
        x264
        fdkaac
        rtmp
        ${log-lib})