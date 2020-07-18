package org.libsdl.app;

import android.content.Intent;
import android.os.Bundle;

import com.example.lfplayer.BaseSelectMediaFileActivity;
import com.example.lfplayer.R;

public class SDLSelectMediaFileActivity extends BaseSelectMediaFileActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sdl_select_media_file);
        findViewById(R.id.select_video_file).setOnClickListener(
                v -> {
                    requestSelectFile();
                }
        );
    }


    @Override
    protected void onPathSelected(String path) {
        startActivity(
                new Intent(this, SDLActivity.class)
                        .putExtra("path", path)
        );
        finish();
    }
}