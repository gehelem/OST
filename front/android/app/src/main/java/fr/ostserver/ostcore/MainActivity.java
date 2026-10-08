package fr.ostserver.ostcore;

import android.os.Bundle;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;
import com.getcapacitor.BridgeActivity;

// Immersive mode - Gilles, 2026-10-05: "je voudrais que l'app masque les
// menus generaux du smartphone/tablette (reseau heure, boutons de
// navigation systeme)" (what Android itself calls "immersive mode", not
// "shadow"). Hides both the status bar (network/time/battery) and the
// navigation bar (back/home/recents); a swipe from the relevant edge
// reveals them temporarily (BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE), then
// they auto-hide again - the standard AndroidX WindowInsetsControllerCompat
// way to do this (no extra Capacitor plugin needed, androidx.core already
// comes in transitively via appcompat). Re-applied on every
// onWindowFocusChanged(true): the bars can get shown back by the system
// (a dialog, the user's own swipe) and Android doesn't restore the hidden
// state on its own once that happens.
public class MainActivity extends BridgeActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        hideSystemBars();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) hideSystemBars();
    }

    private void hideSystemBars() {
        WindowCompat.setDecorFitsSystemWindows(getWindow(), false);
        WindowInsetsControllerCompat controller = WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());
        if (controller != null) {
            controller.hide(WindowInsetsCompat.Type.systemBars());
            controller.setSystemBarsBehavior(WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
        }
    }
}
