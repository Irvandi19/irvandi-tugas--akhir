package com.example.smartmonitoringdancontrollingdatacenter

import android.content.Intent
import android.os.Bundle
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.Switch
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.google.firebase.database.DataSnapshot
import com.google.firebase.database.DatabaseError
import com.google.firebase.database.DatabaseReference
import com.google.firebase.database.FirebaseDatabase
import com.google.firebase.database.ValueEventListener

class MainActivityBeranda : AppCompatActivity() {

    private lateinit var textviewSuhu: TextView
    private lateinit var textviewApi: TextView
    private lateinit var textviewAsap: TextView
    private lateinit var textviewArus: TextView
    private lateinit var switchLamp: Switch
    private lateinit var switchAC: Switch
    private lateinit var switchPintu: Switch
    private lateinit var database: DatabaseReference
    private lateinit var lampAutoImage: ImageView
    private lateinit var lampManualImage: ImageView
    private lateinit var acAutoImage: ImageView
    private lateinit var acManualImage: ImageView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main_beranda)

        // Initialize UI components
        textviewSuhu = findViewById(R.id.textSuhu)
        textviewApi = findViewById(R.id.textApi)
        textviewAsap = findViewById(R.id.textAsap)
        textviewArus = findViewById(R.id.textArus)
        switchLamp = findViewById(R.id.switchLamp)
        switchAC = findViewById(R.id.switchAC)
        switchPintu = findViewById(R.id.switchPintu)
        lampAutoImage = findViewById(R.id.lampAutoImage)
        lampManualImage = findViewById(R.id.lampManualImage)
        acAutoImage = findViewById(R.id.acAutoImage)
        acManualImage = findViewById(R.id.acManualImage)
        val btnBukaKamera = findViewById<LinearLayout>(R.id.btnKamera)
        val tombolPenjadwalan = findViewById<LinearLayout>(R.id.btnJadwal)

        // Navigasi ke aktivitas lain
        btnBukaKamera.setOnClickListener {
            val intentKamera = Intent(this, KameraActivity::class.java)
            startActivity(intentKamera)
        }
        tombolPenjadwalan.setOnClickListener {
            val intentPenjadwalan = Intent(this, PenjadwalanActivity::class.java)
            startActivity(intentPenjadwalan)
        }

        // Inisialisasi referensi database
        database = FirebaseDatabase.getInstance().reference

        // Mendapatkan status terbaru saklar dari Firebase
        val relaysRef = database.child("relays")
        relaysRef.addValueEventListener(object : ValueEventListener {
            override fun onDataChange(dataSnapshot: DataSnapshot) {
                if (dataSnapshot.exists()) {
                    val lampStatus = dataSnapshot.child("saklarLampu").value.toString()
                    val acStatus = dataSnapshot.child("saklarAC").value.toString()
                    val doorStatus = dataSnapshot.child("saklarPintu").value.toString()

                    val lampAutoStatus = dataSnapshot.child("saklarLampuOTO").value.toString()
                    val acAutoStatus = dataSnapshot.child("saklarACoto").value.toString()

                    // Set status saklar sesuai dengan data terbaru dari Firebase
                    switchLamp.isChecked = (lampStatus == "on")
                    switchAC.isChecked = (acStatus == "on")
                    switchPintu.isChecked = (doorStatus == "kunci")

                    // Update image visibility based on auto status
                    if (lampAutoStatus == "on") {
                        lampAutoImage.visibility = ImageView.VISIBLE
                        lampManualImage.visibility = ImageView.GONE
                    } else {
                        lampAutoImage.visibility = ImageView.GONE
                        lampManualImage.visibility = ImageView.VISIBLE
                    }

                    if (acAutoStatus == "on") {
                        acAutoImage.visibility = ImageView.VISIBLE
                        acManualImage.visibility = ImageView.GONE
                    } else {
                        acAutoImage.visibility = ImageView.GONE
                        acManualImage.visibility = ImageView.VISIBLE
                    }
                }
            }

            override fun onCancelled(databaseError: DatabaseError) {
                println("The read failed: " + databaseError.code)
            }
        })

        // Menangani pembaruan data sensor
        database.child("sensors").addValueEventListener(object : ValueEventListener {
            override fun onDataChange(dataSnapshot: DataSnapshot) {
                if (dataSnapshot.exists()) {
                    val suhu = dataSnapshot.child("suhu").value.toString()
                    val api = dataSnapshot.child("api").value.toString()
                    val asap = dataSnapshot.child("asap").value.toString()
                    val arus = dataSnapshot.child("arus").value.toString()

                    // Update TextViews
                    textviewSuhu.text = "$suhu °C"
                    textviewApi.text = api
                    textviewAsap.text = asap

                    // Change status arus to "menyala" or "padam"
                    val arusValue = arus.toFloatOrNull()
                    if (arusValue != null && arusValue < 5) {
                        textviewArus.text = "Padam"
                    } else {
                        textviewArus.text = "Menyala"
                    }
                }
            }

            override fun onCancelled(databaseError: DatabaseError) {
                println("The read failed: " + databaseError.code)
            }
        })

        // Menangani saklar
        switchLamp.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                relaysRef.child("saklarLampu").setValue("on")
            } else {
                relaysRef.child("saklarLampu").setValue("off")
            }
        }

        switchAC.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                relaysRef.child("saklarAC").setValue("on")
            } else {
                relaysRef.child("saklarAC").setValue("off")
            }
        }

        switchPintu.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                relaysRef.child("saklarPintu").setValue("kunci")
            } else {
                relaysRef.child("saklarPintu").setValue("buka")
            }
        }

    }
}
