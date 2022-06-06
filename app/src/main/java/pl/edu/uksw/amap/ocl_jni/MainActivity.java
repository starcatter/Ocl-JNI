package pl.edu.uksw.amap.ocl_jni;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

import pl.edu.uksw.amap.ocl_jni.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    public static final int MULTIPLY_SIZE = 3;

    // Used to load the 'ocl_jni' library on application startup.
    static {
        System.loadLibrary("ocl_jni");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());

        // prepare data arrays
        float[] a = new float[]{1, 2, 3};
        float[] b = new float[]{2, 2, 2};
        float[] result = new float[MULTIPLY_SIZE];

        // Get multiplication result from JNI
        multiplyJni(a, b, result);

        // format output string
        StringBuilder resultString = new StringBuilder();
        for (int i = 0; i < MULTIPLY_SIZE; i++) {
            resultString.append(String.format("%s * %s = %s\n", a[i], b[i], result[i]));
        }

        // Display JNI result
        TextView tvJni = binding.multiplyJniResult;
        tvJni.setText(resultString.toString());

        // change data
        a = new float[]{5, 10, 15};
        b = new float[]{3, 6, 9};

        // Get multiplication result from OCL
        initOCL();
        multiplyOcl(a, b, result);
        shutdownOCL();

        // format output string
        StringBuilder oclResultString = new StringBuilder();
        for (int i = 0; i < MULTIPLY_SIZE; i++) {
            oclResultString.append(String.format("%s * %s = %s\n", a[i], b[i], result[i]));
        }

        // Display multiplyOcl result
        TextView tvOcl = binding.multiplyOclresult;
        tvOcl.setText(oclResultString.toString());
    }

    /**
     * A native method that is implemented by the 'ocl_jni' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    /**
     * Multiply arrays a and b, place result in "result" array
     */
    public native void multiplyJni(float[] a, float[] b, float[] result);

    /**
     * Init/close OpenCL context
     */
    public native void initOCL();
    public native void multiplyOcl(float[] a, float[] b, float[] result);
    public native void shutdownOCL();
}