import android.app.*;
import android.view.*;
import android.os.*;
import android.widget.TextView;
import android.content.DialogInterface;
import android.util.Log;

// TODO: Use AppCompatActivity
//import androidx.core.view.*;
//import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";

	private SurfaceView surfaceView;

    static {
		try {
        	System.loadLibrary("test");
		}
		catch (Exception e) {
			Log.e(TAG, e.getMessage());
		}
    }

	public static native void createGraphics(Surface surface);
    public static native void addTouchEvent(int action, float x, float y);

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
						//	LOGI(String.format("onClick Got %d", i));
						}
					})
					.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
						@Override
						public void onClick(DialogInterface dialogInterface, int i) {
						//	LOGI(String.format("onClick Got %d", i));
						}
					})
					.setCancelable(false)
					.show();
			}
		});
	}

	private void hideSystemUI() {
		View decorView = getWindow().getDecorView();
		int uiOptions = decorView.getSystemUiVisibility();
		int newUiOptions = uiOptions;
		newUiOptions |= View.SYSTEM_UI_FLAG_LOW_PROFILE;
		newUiOptions |= View.SYSTEM_UI_FLAG_FULLSCREEN;
		newUiOptions |= View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
		newUiOptions |= View.SYSTEM_UI_FLAG_IMMERSIVE;
		newUiOptions |= View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
		decorView.setSystemUiVisibility(newUiOptions);
	}

	@Override
	protected void onCreate(Bundle savedInstanceInfo) {
		super.onCreate(savedInstanceInfo);

		// create();
		surfaceView = new SurfaceView(this);
		setContentView(surfaceView);
		hideSystemUI();

		Log.i(TAG, "Hello from Java!");
		surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
			@Override
			public void surfaceCreated(SurfaceHolder holder) {
				createGraphics(holder.getSurface());
			}

			@Override
			public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
			//	resizeGraphics(holder.getSurface(), format, width, height);
			}

			@Override
			public void surfaceDestroyed(SurfaceHolder holder) {
			//	destroyGraphics();
			}
		});
	}

	@Override
	public boolean onTouchEvent(MotionEvent event) {
		super.onTouchEvent(event);
		addTouchEvent(event.getAction(), event.getX(), event.getY());
		return true;
	}
}