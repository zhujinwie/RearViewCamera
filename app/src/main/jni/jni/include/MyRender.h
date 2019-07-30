//
// Created by zjw on 2019/7/30.
//

#ifndef REARVIEWCAMERA_MYRENDER_H
#define REARVIEWCAMERA_MYRENDER_H

#endif //REARVIEWCAMERA_MYRENDER_H

#define TEST_ANDROID
#include <jni.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <unistd.h>

#ifdef TEST_QNX
#include <screen/screen.h>
screen_context_t screen_ctx;
screen_window_t screen_win;
#endif

#ifdef TEST_ANDROID
#include <WindowSurface.h>
using namespace android;
WindowSurface windowSurface;
#endif

#define SCREEN_WIDTH (3)
#define SCREEN_HEIGHT (100)

EGLDisplay egl_display;
EGLContext egl_context;
EGLSurface egl_surface;

static GLint positionLocation;
static GLint texcoordLocation;
static GLint directionLocation;
// static GLint mvpLoc;

static GLuint programObject;

static float m_direction = 0.0;
static float m_flag = 1;

char vShaderStr[] =
"attribute vec3 aPosition;                                                                                                                      \n"
"attribute vec2 texCoord;                                                                                                                       \n"
"varying mediump vec2 uv;                                                                                                                       \n"
"uniform mat4 u_mvpMat;                                                                                                                         \n"
"uniform lowp float direction;                                                                                                                  \n"
"#define PI 3.1415926535897932384626433832795                                                                                                   \n"
"void main()                                                                                                                                    \n"
"{                                                                                                                                              \n"
"    precision mediump float;                                                                                                                   \n"
"    uv = texCoord;                                                                                                                             \n"
"    vec4 transPos = vec4(0.0);                                                                                                                 \n"
"    if (direction > 0.000001)                                                                                                                  \n"
"    {                                                                                                                                          \n"
"        transPos.x = (-cos((1.0 - aPosition.y) * direction * PI / 2.0) * (1.0 / direction - 1.0 + aPosition.x) + 1.0 / direction - 0.5)*1.5;       \n"
"        transPos.y = (sin((1.0 - aPosition.y) * direction * PI / 2.0) * (1.0 / direction - 1.0 + aPosition.x)- 1.0);                           \n"
"        transPos.z = aPosition.z;                                                                                                              \n"
"    }                                                                                                                                          \n"
"    else if (direction < -0.00001)                                                                                                             \n"
"    {                                                                                                                                          \n"
"        transPos.x = (cos((1.0 - aPosition.y) * (-direction) * PI / 2.0) * (1.0 / (-direction) - 1.0 + aPosition.x) + 1.0 / direction + 0.5)*1.5;  \n"
"        transPos.y = (sin((1.0 - aPosition.y) * (-direction) * PI / 2.0) * (1.0 / (-direction) - 1.0 + aPosition.x)- 1.0);                     \n"
"        transPos.z = aPosition.z;                                                                                                              \n"
"    }                                                                                                                                          \n"
"    else                                                                                                                                       \n"
"    {                                                                                                                                          \n"
"        float tmpDirection = -0.0001;\n"
"        transPos.x = (cos((1.0 - aPosition.y) * (-tmpDirection) * PI / 2.0) * (1.0 / (-tmpDirection) - 1.0 + aPosition.x) + 1.0 / tmpDirection + 0.5)*1.5; \n"
"        transPos.y = (sin((1.0 - aPosition.y) * (-tmpDirection) * PI / 2.0) * (1.0 / (-tmpDirection) - 1.0 + aPosition.x)- 1.0);               \n"
"        transPos.z = aPosition.z;                                                                                                              \n"
"         // transPos.x = 2.0*(aPosition.x - 0.5);\n"
"         // transPos.y = 2.0*(1.0 - aPosition.y - 0.5);\n"
"         // transPos.z = aPosition.z; \n"
"    }                                                                                                                                          \n"
"    gl_Position = vec4(transPos.x, transPos.y, transPos.z, 1.0);                                                                               \n"
"}                                                                                                                                              \n";

char fShaderStr[] =
"varying mediump vec2 uv;                                                                                     \n"
"void main()                                                                                                  \n"
"{                                                                                                            \n"
"    precision lowp float;                                                                                    \n"
"    vec4 color = vec4(0.0, 0.0, 0.0, 0.0);                                                                   \n"
"    if ((uv.y < 0.34 && (uv.x > 0.97 || uv.x < 0.03))||(uv.y < 0.34 && uv.y > 0.31)) {                         \n"
"       color = vec4(1.0, 0.0, 0.0, 1.0);                                                                     \n"
"    }                                                                                                        \n"
"    if (((uv.y > 0.35 && uv.y < 0.69) && (uv.x > 0.97 || uv.x < 0.03))||(uv.y < 0.69 && uv.y > 0.66)) {       \n"
"       color = vec4(1.0, 1.0, 0.0, 1.0);                                                                     \n"
"    }                                                                                                        \n"
"    if (((uv.y > 0.7 && uv.y < 1.0) && (uv.x > 0.97 || uv.x < 0.03))||(uv.y < 1.00 && uv.y > 0.97)) {         \n"
"       color = vec4(0.0, 1.0, 0.0, 1.0);                                                                     \n"
"    }                                                                                                        \n"
"    if (uv.x > 0.3 && uv.x < 0.7 && uv.y < 0.97) {                                                                          \n"
"       color = vec4(0.0);                                                                                    \n"
"    }                                                                                                        \n"
"    if (uv.x > 0.985) { color.a = (1.0 - uv.x) * 200.0; }"
"    if (uv.x < 0.015) { color.a = uv.x * 200.0; }"
"    if (uv.y > 0.985) { color.a = (1.0 - uv.y) * 200.0; }"
"    if (uv.x > 0.985 && uv.y > 0.985) { color.a = (1.0 - uv.y) * 200.0 * (1.0 - uv.x) * 65.0; }"
"    if (uv.x < 0.015 && uv.y > 0.985) { color.a = uv.x * 200.0 * (1.0 - uv.y) * 65.0; }"

"    if (uv.x < 0.985 && uv.x > 0.015 && uv.y < 0.69 && uv.y > 0.675) { color.a = (0.69 - uv.y) * 200.0; }"
"    if (uv.x > 0.985 && uv.y < 0.69 && uv.y > 0.675) { color.a = (0.69 - uv.y) * 200.0 * (1.0 - uv.x) * 65.0; }"
"    if (uv.x < 0.015 && uv.y < 0.69 && uv.y > 0.675) { color.a = (0.69 - uv.y) * 200.0 * uv.x * 100.0; }"

"    if (uv.x < 0.97 && uv.x > 0.03 && uv.y < 0.985 && uv.y > 0.97) { color.a = (uv.y - 0.97) * 200.0; }"
"    if (uv.x < 0.97 && uv.x > 0.03 && uv.y < 0.675 && uv.y > 0.66) { color.a = (uv.y - 0.66) * 200.0; }"
"    if (uv.x < 0.97 && uv.x > 0.03 && uv.y < 0.325 && uv.y > 0.31) { color.a = (uv.y - 0.31) * 200.0; }"
"    if (uv.x < 0.985 && uv.x > 0.015 && uv.y < 0.34 && uv.y > 0.325) { color.a = (0.34 - uv.y) * 200.0; }"
"    if (uv.x > 0.985 && uv.y < 0.34 && uv.y > 0.325) { color.a = (0.34 - uv.y) * 200.0 * (1.0 - uv.x) * 65.0; }"
"    if (uv.x < 0.015 && uv.y < 0.34 && uv.y > 0.325) { color.a = (0.34 - uv.y) * 200.0 * uv.x * 65.0; }"

"    if (uv.x > 0.015 && uv.x < 0.03 && (uv.y < 0.31 || (uv.y > 0.34 && uv.y < 0.66) || (uv.y > 0.69 && uv.y < 0.97) )) { color.a = (0.03 - uv.x) * 300.0;}"
"    if (uv.x > 0.97 && uv.x < 0.985 && (uv.y < 0.31 || (uv.y > 0.34 && uv.y < 0.66) || (uv.y > 0.69 && uv.y < 0.97) )) { color.a = (uv.x - 0.97) * 300.0;}"
"    gl_FragColor.rgba = color.rgba;                                                                          \n"
"}                                                                                                            \n";



GLfloat position[SCREEN_WIDTH * SCREEN_HEIGHT * 3];
GLfloat texcoord[SCREEN_WIDTH * SCREEN_HEIGHT * 2];
GLuint element[(SCREEN_WIDTH - 1) * (SCREEN_HEIGHT - 1) * 2 * 3];