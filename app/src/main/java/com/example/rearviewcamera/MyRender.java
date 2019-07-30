package com.example.rearviewcamera;

import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MyRender implements GLSurfaceView.Renderer {

    private static final String TAG = "agl";

    private native void nativeDrawFrame();

    private native void nativeSurfaceChanged(int width, int height);

    private native void nativeSurfaceCreated();

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        nativeSurfaceCreated();
        Log.d(TAG, "onDrawFrame " );
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int i, int i1) {
        nativeSurfaceChanged(i,i1);
        Log.d(TAG, "onSurfaceChanged");
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        nativeDrawFrame();
        Log.d(TAG, "onSurfaceCreate");
    }
}
