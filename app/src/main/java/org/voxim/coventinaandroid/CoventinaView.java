package org.voxim.coventinaandroid;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class CoventinaView extends GLSurfaceView {
    public native void init();
    public native void resize(int width, int height);
    public native void redraw();

    private long startTime;

    public CoventinaView(Context ctx) {
        super(ctx);

        setEGLConfigChooser(8, 8, 8, 0, 16, 0);
        setEGLContextClientVersion(3);
        setPreserveEGLContextOnPause(true);
        setRenderer(new RendererWrapper());
    }

    public class RendererWrapper implements GLSurfaceView.Renderer {
        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            Log.d("Coventina View", "onSurfaceCreated");
            gl.glClearColor(0, 0, 1, 0);
            gl.glClear(GL10.GL_COLOR_BUFFER_BIT);

            resize(getWidth(), getHeight());
            init();
            startTime = System.currentTimeMillis();
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            resize(width, height);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            long endTime = System.currentTimeMillis();
            long dt = endTime - startTime;

            if (dt < 33) {
                try {
                    Thread.sleep(33 - dt);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            startTime = System.currentTimeMillis();

        // ###########
            redraw();
        // ###########

        }
    }
}
