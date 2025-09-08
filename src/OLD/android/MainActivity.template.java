package $namespace;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;

public class MainActivity extends AppCompatActivity {

	private SurfaceView surfaceView;
	private static final String TAG = "MainActivity";

	public native int create();
	public native int destroy();

	public static native void createGraphics(Surface surface);
	public static native void resizeGraphics(Surface surface, int format, int width, int height);
	public static native void destroyGraphics();

	public static native void addTouchEvent(int action, float x, float y);
	
	static {
		try {
			System.loadLibrary("$name");
		}
		catch (Exception e) {
			LOGE("Error: " + e.getMessage());
		}
	}

	public void showDialog(String title, String message) {
		final MainActivity activity = this;
		runOnUiThread(new Runnable() {
			@Override
			public void run() {
				new AlertDialog.Builder(activity)
					.setTitle(title)
					.setMessage(message)
					.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
						@Override
						public void onClick(DialogInterface dialogInterface, int i) {
							LOGI(String.format("onClick Got %d", i));
						}
					})
					.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
						@Override
						public void onClick(DialogInterface dialogInterface, int i) {
							LOGI(String.format("onClick Got %d", i));
						}
					})
					.setCancelable(false)
					.show();
			}
		});
	}

	private void hideSystemUI() {
		WindowCompat.setDecorFitsSystemWindows(getWindow(), false);
		WindowInsetsControllerCompat insetsController = WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());
		insetsController.setSystemBarsBehavior(WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
		insetsController.hide(WindowInsetsCompat.Type.statusBars());
		insetsController.hide(WindowInsetsCompat.Type.navigationBars());
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		super.onTouchEvent(event);
		String action = "Unknown";
		switch (event.getAction()) {
			case MotionEvent.ACTION_DOWN:         action = "DOWN";         break;
			case MotionEvent.ACTION_UP:           action = "UP";           break;
			case MotionEvent.ACTION_POINTER_DOWN: action = "POINTER_DOWN"; break;
			case MotionEvent.ACTION_POINTER_UP:   action = "POINTER_UP";   break;
			case MotionEvent.ACTION_MOVE:         action = "MOVE";         break;
		}
		//Log.v(TAG, String.format("Got Touch Event (%.2f,%.2f) action=%s", event.getX(), event.getY(), action));
		addTouchEvent(event.getAction(), event.getX(), event.getY());
		return true;
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus) {
			hideSystemUI();
		}
	}

	@Override
	protected void onCreate(Bundle savedInstanceInfo) {
		super.onCreate(savedInstanceInfo);

		create();

		surfaceView = new SurfaceView(this);

		setContentView(surfaceView);

		hideSystemUI();

		surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
			@Override
			public void surfaceCreated(SurfaceHolder holder) {
				createGraphics(holder.getSurface());
			}

			@Override
			public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
				resizeGraphics(holder.getSurface(), format, width, height);
			}

			@Override
			public void surfaceDestroyed(SurfaceHolder holder) {
				destroyGraphics();
			}
		});

        LOGI("Hello from Java!");
	}

	@Override
	protected void onDestroy() {
        LOGI("BYE! from Java!");
		destroy();
		super.onDestroy();
	}

	private static void LOGI(String message) {
		Log.i(TAG, "[Java] " + message);
	}
	private static void LOGE(String message) {
		Log.e(TAG, "[Java] " + message);
	}
}
