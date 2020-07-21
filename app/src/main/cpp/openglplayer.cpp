//
// Created by 79852 on 2020/7/20.
//

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <player/lfplayerutils.h>
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

void init(JNIEnv *env, jobject javeSurface) {
    EGLDisplay eglDisplay;
    if ((eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        LOG("eglGetDisplay get error:%d", eglGetError());
        return;
    }
    if (!eglInitialize(eglDisplay, nullptr, nullptr)) {
        LOG("eglInitialize get error:%d", eglGetError());
        return;
    }
    const EGLint attribs[] = {
            EGL_BUFFER_SIZE, 32,
            EGL_ALPHA_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_NONE
    };
    EGLConfig eglConfig;
    EGLint elgConfigNums;
    if (!eglChooseConfig(eglDisplay, attribs, &eglConfig, 1, &elgConfigNums)) {
        LOG("eglChooseConfig get error:%d", eglGetError());
        return;
    }
    EGLint contextAttribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };
    EGLContext eglContext;
    if (!(eglContext = eglCreateContext(eglDisplay, eglConfig, nullptr, contextAttribs))) {
        LOG("eglCreateContext get error:%d", eglGetError());
        return;
    }
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, javeSurface);
    EGLint format;
    eglGetConfigAttrib(eglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(nativeWindow, 0, 0, format);
    EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, nullptr);
    if (!eglSurface) {
        LOG("eglCreateWindowSurface get error: %d", eglGetError());
        return;
    }
    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        LOG("eglMakeCurrent get error: %d", eglGetError());
        return;
    }
    LOG("native egl init complete");
}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_lfplayer_NativeOpenGLActivity_nativeSurfaceCreated(JNIEnv *env, jobject thiz,
                                                              jobject surface) {
    init(env, surface);
}

