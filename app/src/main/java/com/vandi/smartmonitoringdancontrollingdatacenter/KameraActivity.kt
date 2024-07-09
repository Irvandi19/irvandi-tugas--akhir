package com.vandi.smartmonitoringdancontrollingdatacenter

import android.content.Intent
import android.os.Bundle
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import com.github.niqdev.mjpeg.DisplayMode
import com.github.niqdev.mjpeg.Mjpeg
import com.github.niqdev.mjpeg.MjpegSurfaceView

class KameraActivity : AppCompatActivity() {

    private lateinit var mjpegSurfaceView: MjpegSurfaceView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_kamera)

        mjpegSurfaceView = findViewById(R.id.mjpegViewDefault)

        // Replace with your ESP32-CAM URL
        val url = "http://192.168.58.172:81/stream"

        Mjpeg.newInstance()
            .open(url, 2) // Timeout after 5 seconds
            .subscribe(
                { inputStream ->
                    mjpegSurfaceView.setSource(inputStream)
                    mjpegSurfaceView.setDisplayMode(DisplayMode.BEST_FIT)
                    mjpegSurfaceView.showFps(true)
                },
                { throwable ->
                    throwable.printStackTrace()
                }
            )

        val kembali = findViewById<ImageView>(R.id.btnBack1)

        kembali.setOnClickListener {
            val intentKamera = Intent(this, MainActivityBeranda::class.java)
            startActivity(intentKamera)
        }
    }
}
