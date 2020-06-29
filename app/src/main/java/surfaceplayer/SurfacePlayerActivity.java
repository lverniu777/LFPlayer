package surfaceplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;

import com.example.lfplayer.R;

import butterknife.OnClick;

public class SurfacePlayerActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_surface_player);
        final SurfaceView surfaceView = findViewById(R.id.surface_view);
        surfaceView.getHolder().addCallback(new SurfaceCallback(this));
    }

    @OnClick(R.id.surface_play)
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.surface_play:
                break;
        }
    }
}