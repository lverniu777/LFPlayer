package org.libsdl.app;

import android.content.ClipboardManager;
import android.content.Context;

class SDLClipboardHandler_API11 implements
        SDLClipboardHandler,
        ClipboardManager.OnPrimaryClipChangedListener {

    protected ClipboardManager mClipMgr;

    SDLClipboardHandler_API11() {
        mClipMgr = (ClipboardManager) SDL.getContext().getSystemService(Context.CLIPBOARD_SERVICE);
        mClipMgr.addPrimaryClipChangedListener(this);
    }

    @Override
    public boolean clipboardHasText() {
        return mClipMgr.hasText();
    }

    @Override
    public String clipboardGetText() {
        CharSequence text;
        text = mClipMgr.getText();
        if (text != null) {
            return text.toString();
        }
        return null;
    }

    @Override
    public void clipboardSetText(String string) {
        mClipMgr.removePrimaryClipChangedListener(this);
        mClipMgr.setText(string);
        mClipMgr.addPrimaryClipChangedListener(this);
    }

    @Override
    public void onPrimaryClipChanged() {
        SDLActivity.onNativeClipboardChanged();
    }

}
