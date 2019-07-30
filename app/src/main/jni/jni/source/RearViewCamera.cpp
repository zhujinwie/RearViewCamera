#include <com_example_rearviewcamera_MyRender.h>

static int reflashMesh()
{
  memset(position, 0, sizeof(position));
  memset(texcoord, 0, sizeof(texcoord));
  memset(element, 0, sizeof(element));

  for (int i = 0; i < SCREEN_HEIGHT; ++i) {
    for (int j = 0; j < SCREEN_WIDTH; ++j) {
      position[(i * SCREEN_WIDTH + j) * 3 + 0] = (float)j / (float)SCREEN_WIDTH;
      position[(i * SCREEN_WIDTH + j) * 3 + 1] = (float)i / (float)(SCREEN_HEIGHT);
      position[(i * SCREEN_WIDTH + j) * 3 + 2] = 0.0f;
      texcoord[(i * SCREEN_WIDTH + j) * 2 + 0] = (float)j / (float)SCREEN_WIDTH;
      texcoord[(i * SCREEN_WIDTH + j) * 2 + 1] = (float)(SCREEN_HEIGHT - (i + 1)) / (float)(SCREEN_HEIGHT);

    }
  }

  for (int i = 0; i < SCREEN_HEIGHT - 1; ++i) {
    for (int j = 0; j < SCREEN_WIDTH - 1; ++j) {
      element[(i * (SCREEN_WIDTH - 1) + j) * 6 + 0] = i * SCREEN_WIDTH + j;
      element[(i * (SCREEN_WIDTH - 1) + j) * 6 + 1] = (i + 1) * SCREEN_WIDTH + j;
      element[(i * (SCREEN_WIDTH - 1) + j) * 6 + 2] = i * SCREEN_WIDTH + j + 1;

      element[(i * (SCREEN_WIDTH - 1) + j) * 6 + 3] = (i + 1) * SCREEN_WIDTH + j;
      element[(i * (SCREEN_WIDTH - 1) + j) * 6 + 4] = (i + 1) * SCREEN_WIDTH + j + 1;
      element[(i * (SCREEN_WIDTH - 1) + j) * 6 + 5] = i * SCREEN_WIDTH + j + 1;
    }
  }
  return EXIT_SUCCESS;
}

#ifdef TEST_QNX
static int initScreen(void)
{
    int rc;

    // Create the screen context
    rc = screen_create_context(&screen_ctx, SCREEN_APPLICATION_CONTEXT);
    if (rc) {
        perror("screen_create_context");
        return EXIT_FAILURE;
    }

    // Create the screen window that will be render onto
    rc = screen_create_window(&screen_win, screen_ctx);
    if (rc) {
        perror("screen_create_window");
        return EXIT_FAILURE;
    }

    char new_name[64] = "DPY_CARLINE";
    screen_set_window_property_cv(screen_win, SCREEN_PROPERTY_ID_STRING, sizeof(new_name), new_name);

    // Set the value of the specified window property of type integer
    screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_FORMAT, (const int[]){ SCREEN_FORMAT_RGBX8888 });
    screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_USAGE, (const int[]){ SCREEN_USAGE_OPENGL_ES2 });

    // Send a request to the composition manager to add new buffers to a window
    rc = screen_create_window_buffers(screen_win, 2);
    if (rc) {
        perror("screen_create_window_buffers");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
#endif

static int initEGL(void)
{
    EGLBoolean rc;

    const EGLint egl_attrib_list[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 0,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SAMPLE_BUFFERS, 1,
        EGL_NONE
    };

    EGLConfig egl_conf = (EGLConfig)0;
    EGLint num_confs = 0;

    const EGLint egl_ctx_attr[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    #ifdef TEST_ANDROID
    EGLNativeWindowType screen_window = windowSurface.getSurface();
    #endif
    egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (egl_display == EGL_NO_DISPLAY) {
        fprintf(stderr, "eglGetDisplay failed\n");
        return EXIT_FAILURE;
    }

    rc = eglInitialize(egl_display, NULL, NULL);
    if (rc != EGL_TRUE) {
        fprintf(stderr, "eglInitialize failed\n");
        return EXIT_FAILURE;
    }

    rc = eglChooseConfig(egl_display, egl_attrib_list, &egl_conf, 1, &num_confs);
    if ((rc != EGL_TRUE) || (num_confs == 0)) {
        fprintf(stderr, "eglChooseConfig failed\n");
        return EXIT_FAILURE;
    }

    egl_context = eglCreateContext(egl_display, egl_conf, EGL_NO_CONTEXT, (EGLint*)&egl_ctx_attr);
    if (egl_context == EGL_NO_CONTEXT) {
        fprintf(stderr, "eglCreateContext failed\n");
        return EXIT_FAILURE;
    }

    //Create the EGL surface from the screen window
    egl_surface = eglCreateWindowSurface(egl_display, egl_conf, screen_window, NULL);
    if (egl_surface == EGL_NO_SURFACE) {
        fprintf(stderr, "eglCreateWindowSurface failed\n");
        return EXIT_FAILURE;
    }

    rc = eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
    if (rc != EGL_TRUE) {
        fprintf(stderr, "eglMakeCurrent failed\n");
        return EXIT_FAILURE;
    }

    rc = eglSwapInterval(egl_display, 0);
    if (rc != EGL_TRUE) {
        fprintf(stderr, "eglSwapInterval failed\n");
        // return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

// Create a shader object, load the shader source, and compile the shader
GLuint LoadShader(EGLenum type, const char* shaderSrc)
{
    GLuint shader = 0;
    GLint compiled;

    shader = glCreateShader(type);
    if (!shader) {
        return 0;
    }

    // Load the shader source
    glShaderSource(shader, 1, &shaderSrc, 0);
    // Compile the shader
    glCompileShader(shader);
    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char* infoLog = (char*)malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            fprintf(stderr, "%s\n", infoLog);
            free(infoLog);
        }

        glDeleteShader(shader);
        return 0;
   }

    return shader;
}

static int initOpenGL(void)
{
    GLuint vertexShader;
    GLuint fragmentShader;
    GLint linked;

    vertexShader = LoadShader(GL_VERTEX_SHADER, vShaderStr);
    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fShaderStr);

    programObject = glCreateProgram();

    if (programObject == 0) {
        return 0;
    }

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    // Link the program
    glLinkProgram(programObject);

    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLint infoLen = 0;

        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char* infoLog = (char*)malloc(sizeof(char) * infoLen);
            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            fprintf(stderr, "%s\n", infoLog);
            free(infoLog);
        }

        glDeleteProgram (programObject);
        return EXIT_FAILURE;
    }

    positionLocation = glGetAttribLocation(programObject, "aPosition");
    texcoordLocation = glGetAttribLocation(programObject, "texCoord");
    // mvpLoc = glGetUniformLocation(programObject, "uMvp");
    directionLocation = glGetUniformLocation(programObject, "direction");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    return EXIT_SUCCESS;
}

void render()
{
    EGLint surface_width;
    EGLint surface_height;

    eglQuerySurface(egl_display, egl_surface, EGL_WIDTH, &surface_width);
    eglQuerySurface(egl_display, egl_surface, EGL_HEIGHT, &surface_height);

    // Set the viewport
    // printf("---------------------------%d x %d\n", surface_width, surface_height);

    glViewport(0, 0, 600, 1000);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the program object
    glUseProgram(programObject);

    // Load the vertex position
    //glBindBuffer(GL_ARRAY_BUFFER, vboPosition);
    glEnableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, position);

    // Load the texture coordinate
    // glBindBuffer(GL_ARRAY_BUFFER, vboTexcoord);
    glEnableVertexAttribArray(texcoordLocation);
    glVertexAttribPointer(texcoordLocation, 2, GL_FLOAT, GL_FALSE, 0, texcoord);

    // Index
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboElement);

    // Set the uniform
    glUniform1f(directionLocation, m_direction);

	  //glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvp);


    glDrawElements(GL_TRIANGLES, (SCREEN_WIDTH - 1) * (SCREEN_HEIGHT - 1) * 2 * 3, GL_UNSIGNED_INT, element);

    glDisableVertexAttribArray(positionLocation);
    glDisableVertexAttribArray(texcoordLocation);
}

int main(void)
{
    int rc;

    #ifdef TEST_QNX
    rc = initScreen();
    if (rc != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
    #endif

    rc = reflashMesh();
    if (rc != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    rc = initEGL();
    if (rc != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    rc = initOpenGL();
    if (rc != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    while (1) {
        // sleep(1);
        if (m_direction > 0.50 || m_direction < -0.50) {
            m_flag = -m_flag;
        }
        m_direction = m_direction + 0.005 * m_flag;
        // printf("direction = %f\n", m_direction);
        // m_direction = 0.5;
        // reflashMesh();

        render();

        rc = eglSwapBuffers(egl_display, egl_surface);
        if (rc != EGL_TRUE) {
            fprintf(stderr, "eglSwapBuffers failed\n");
            // break;
        }
    }

    //Clean up
    eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(egl_display, egl_surface);
    eglDestroyContext(egl_display, egl_context);
    eglTerminate(egl_display);
    eglReleaseThread();

    #ifdef TEST_QNX
    screen_destroy_window(screen_win);
    screen_destroy_context(screen_ctx);
    #endif

    return EXIT_SUCCESS;
}

void init()
{
    int rc;

        #ifdef TEST_QNX
        rc = initScreen();
        if (rc != EXIT_SUCCESS) {
            return ;
        }
        #endif

        rc = reflashMesh();
        if (rc != EXIT_SUCCESS) {
            return ;
        }

        rc = initEGL();
        if (rc != EXIT_SUCCESS) {
            return ;
        }

        rc = initOpenGL();
        if (rc != EXIT_SUCCESS) {
            return ;
        }

}

void nativeSurfaceCreated(JNIEnv *env,jobject obj)
{
    init();
}

void nativeSurfaceChanged(JNIEnv *env, jobject obj, jint width, jint height)
{
    initScreen();
}

void nativeDrawFrame(JNIEnv *env,jobject obj)
{
   if (m_direction > 0.50 || m_direction < -0.50) {
               m_flag = -m_flag;
           }
           m_direction = m_direction + 0.005 * m_flag;
           // printf("direction = %f\n", m_direction);
           // m_direction = 0.5;
           // reflashMesh();

           render();

           if (eglSwapBuffers(egl_display, egl_surface) != EGL_TRUE) {
               fprintf(stderr, "eglSwapBuffers failed\n");
               // break;
           }
}

static JNINativeMethod gMethods[] = {
      {"nativeDrawFrame","()V",(void*)nativeDrawFrame},
      {"nativeSurfaceChanged","(II)V",(void*)nativeSurfaceChanged},
      {"nativeSurfaceCreated","()V",(void*)nativeSurfaceCreated}
};


static const char* className="com/example/rearviewcameradynamicjni/MyRenderer";

static int registerNativeMethods(JNIEnv *env) {
	jclass clazz;
	clazz = env->FindClass(className);
	if (clazz == NULL) {
		LOGD("failed to load the class %s", className);
		return JNI_FALSE;
	}
	if (env->RegisterNatives(clazz, gMethods, sizeof(gMethods)/sizeof(gMethods[0])) < 0) {
		return JNI_FALSE;
	}
	return JNI_TRUE;
} //end of registerNativeMethods

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	jint result = -1;
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
	{
		LOGE("ERROR: GetEnv failed\n");
		goto bail;
	}

	if (registerNativeMethods(env) < 0) {
		LOGE("ERROR: jnitest native registration failed\n");
		goto bail;
	}
	result = JNI_VERSION_1_4;

	bail:
	return result;
}

