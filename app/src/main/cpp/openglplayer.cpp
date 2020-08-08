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
#include <cstdlib>
#include <android/bitmap.h>

typedef GLfloat pDouble[8];
const char *gVertexShader =
        "attribute vec4 position;\n"
        "attribute vec2 texcoord;\n"
        "varying vec2 v_texcoord;\n"
        "\n"
        "void main() {\n"
        "    gl_Position = position;\n"
        "    v_texcoord = texcoord;\n"
        "}";

const char *gFragmentShader =
        "precision highp float;\n"
        "varying highp vec2 v_texcoord;\n"
        "uniform sampler2D texSampler;\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(texSampler, v_texcoord);\n"
        "}";

EGLContext eglContext;
EGLSurface eglSurface;

const GLfloat vertex[] = {
        -1.0, -1.0,
        1.0, -1.0,
        -1.0, 0.0,
        1.0, 0.0
};

const GLfloat textureCoordinate[] = {
        0.0, 0.0,
        1.0, 0.0,
        0.0, 1.0,
        1.0, 1.0
};
const GLfloat gTriangleVertices[] = {
        0.0f, 0.5f,
        -0.5f, -0.5f,
        0.5f, -0.5f
};
GLuint gProgram;
GLint gvPositionHandle;

void drawTriangle();

GLuint loadShader(GLenum shaderType, const char *pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char *buf = (char *) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOG("Could not compile shader %d:\n%s\n",
                        shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char *pVertexSource, const char *pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, pixelShader);
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char *buf = (char *) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOG("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}


void drawTriangle() {
    static float grey;
    grey += 0.01f;
    if (grey > 1.0f) {
        grey = 0.0f;
    }
    glClearColor(grey, grey, grey, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glUseProgram(gProgram);

    glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
    glEnableVertexAttribArray(gvPositionHandle);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

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
    if (!(eglContext = eglCreateContext(eglDisplay, eglConfig, nullptr, contextAttribs))) {
        LOG("eglCreateContext get error:%d", eglGetError());
        return;
    }
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, javeSurface);
    EGLint format;
    eglGetConfigAttrib(eglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(nativeWindow, 0, 0, format);
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, nullptr);
    if (!eglSurface) {
        LOG("eglCreateWindowSurface get error: %d", eglGetError());
        return;
    }
    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext)) {
        LOG("eglMakeCurrent get error: %d", eglGetError());
        return;
    }
    LOG("native egl init complete");
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOG("Could not create program");
        return;
    }
    int viewPortWidth = ANativeWindow_getWidth(nativeWindow);
    int viewPortHeight = ANativeWindow_getHeight(nativeWindow);
    glViewport(0, 0, viewPortWidth, viewPortHeight);

    glUseProgram(gProgram);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    GLint position = glGetAttribLocation(gProgram, "position");
    glVertexAttribPointer(position, 2, GL_FLOAT, 0, 0, vertex);
    glEnableVertexAttribArray(position);
    GLint textPosition = glGetAttribLocation(gProgram, "texcoord");
    glVertexAttribPointer(textPosition, 2, GL_FLOAT, 0, 0, textureCoordinate);
    glEnableVertexAttribArray(textPosition);
    GLint textureUniform = glGetUniformLocation(gProgram, "texSampler");

}


extern "C"
JNIEXPORT void JNICALL
Java_com_example_lfplayer_NativeOpenGLActivity_nativeSurfaceCreated(JNIEnv *env, jobject thiz,
                                                                    jobject surface) {
    init(env, surface);
}

