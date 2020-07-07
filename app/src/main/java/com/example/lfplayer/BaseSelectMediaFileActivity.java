package com.example.lfplayer;

import android.app.Activity;
import android.content.Intent;
import android.widget.Toast;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.example.lfplayer.utils.FileUtils;

public abstract class BaseSelectMediaFileActivity extends AppCompatActivity {

    private static final int CHOOSE_FILE_REQUEST_CODE = 6;

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != CHOOSE_FILE_REQUEST_CODE) {
            return;
        }
        if (resultCode != Activity.RESULT_OK) {
            return;
        }
        if (data == null || data.getData() == null) {
            Toast.makeText(this, "没有选中文件", Toast.LENGTH_LONG).show();
        } else {
            final String filePath = FileUtils.INSTANCE.getPath(this, data.getData());
            onPathSelected(filePath);
        }
    }

    protected void requestSelectFile() {
        final Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*");//无类型限制
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        startActivityForResult(intent, CHOOSE_FILE_REQUEST_CODE);
    }

    protected abstract void onPathSelected(String path);
}
