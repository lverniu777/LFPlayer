package com.example.lfplayer.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.camera.view.PreviewView;

/**
 * TODO 使用CameraX进行视频采集
 */
public class CameraXView extends FrameLayout implements ICameraView {
    private PreviewView mPreviewView;

    public CameraXView(@NonNull Context context) {
        this(context, null);
    }

    public CameraXView(@NonNull Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CameraXView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        this(context, attrs, defStyleAttr, 0);
    }

    public CameraXView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        final ViewGroup.LayoutParams vlp = new FrameLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT);
        mPreviewView = new PreviewView(context, attrs, defStyleAttr, defStyleRes);
        addView(mPreviewView, vlp);
    }
}
