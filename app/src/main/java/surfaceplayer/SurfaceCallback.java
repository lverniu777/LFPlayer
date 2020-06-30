package surfaceplayer;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.SurfaceHolder;

import com.example.lfplayer.R;

public class SurfaceCallback implements SurfaceHolder.Callback {
    private SurfaceHolder mSurfaceHolder;

    private Context mContext;

    public SurfaceCallback(Context context) {
        this.mContext = context;
    }

    static {
        System.loadLibrary("lfplayer");
    }


    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        mSurfaceHolder = holder;
        new Thread(() -> {
            nativePlay("");
        }
        ).start();
    }

    private native void nativePlay(String playPath);


    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }
}
