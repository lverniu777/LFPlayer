package com.example.lfplayer.view;

import android.content.Context;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Build;
import android.util.AttributeSet;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;

import androidx.annotation.RequiresApi;

import java.io.IOException;
import java.util.List;

import static android.hardware.Camera.CameraInfo.CAMERA_FACING_BACK;

public class CameraSurfaceView extends SurfaceView implements SurfaceHolder.Callback, Camera.PreviewCallback {
    private Camera mCamera;
    private static final String TAG = CameraSurfaceView.class.getSimpleName();

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public CameraSurfaceView(Context context) {
        this(context, null);
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public CameraSurfaceView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public CameraSurfaceView(Context context, AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public CameraSurfaceView(Context context, AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        getHolder().addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        if (mCamera != null) {
            mCamera.stopPreview();
            mCamera.release();
        }
        mCamera = Camera.open(CAMERA_FACING_BACK);
        try {
            mCamera.setPreviewDisplay(holder);
            setCameraParam(getMeasuredWidth(), getMeasuredHeight());
            mCamera.startPreview();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private void setCameraParam(int width, int height) {
        Camera.Parameters parameters = mCamera.getParameters();
        final WindowManager windowManager = ((WindowManager) getContext().getSystemService(Context.WINDOW_SERVICE));
        if (windowManager == null) {
            return;
        }
        final int angle = windowManager.getDefaultDisplay().getRotation();
        switch (angle) {
            case Surface.ROTATION_0:
                mCamera.setDisplayOrientation(90);
                break;
            case Surface.ROTATION_90:
                mCamera.setDisplayOrientation(0);
                break;
            case Surface.ROTATION_180:
                mCamera.setDisplayOrientation(270);
                break;
            case Surface.ROTATION_270:
                mCamera.setDisplayOrientation(180);
                break;
        }
        parameters.setFlashMode("off");
        parameters.setPreviewFormat(ImageFormat.NV21);
        Camera.Size size = getFitSize(width, height, parameters.getSupportedPictureSizes());
        parameters.setPictureSize(size.width, size.height);
        size = getFitSize(width, height, parameters.getSupportedPreviewSizes());
        parameters.setPreviewSize(1920, 1080);
        mCamera.setParameters(parameters);
    }

    private Camera.Size getFitSize(int width, int height, List<Camera.Size> sizes) {
        if (width < height) {
            int t = height;
            height = width;
            width = t;
        }

        for (Camera.Size size : sizes) {
            if (1.0f * size.width / size.height == 1.0f * width / height) {
                return size;
            }
        }
        return sizes.get(0);
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (holder == null || holder.getSurface() == null || !holder.getSurface().isValid()) {
            return;
        }
        if (mCamera != null) {
            mCamera.stopPreview();
            try {
                mCamera.setPreviewDisplay(holder);
                mCamera.setPreviewCallback(this);
                setCameraParam(width, height);
                mCamera.startPreview();
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        if (mCamera != null) {
            mCamera.stopPreview();
            mCamera.release();
        }
        mCamera = null;
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        Log.e(TAG, "onPreviewFrame : " + data.length + " data: " + data);
    }
}
