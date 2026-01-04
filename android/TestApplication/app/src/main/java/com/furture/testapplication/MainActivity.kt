package com.furture.testapplication

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import com.furture.testapplication.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method

        binding.sampleText.text = stringFromJNI()
        binding.sampleText.setOnClickListener {
            System.loadLibrary("crypto")
            System.loadLibrary("ssl")
            System.loadLibrary("crypto2")
            System.loadLibrary("testapplication")
            binding.sampleText.text = stringFromJNI() + "ddd"
        }
    }

    /**
     * A native method that is implemented by the 'testapplication' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

    companion object {
        // Used to load the 'testapplication' library on application startup.
        init {
            System.loadLibrary("crypto")
            System.loadLibrary("ssl")
            System.loadLibrary("testapplication")
        }
    }
}