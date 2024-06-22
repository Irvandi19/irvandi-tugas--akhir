package com.example.smartmonitoringdancontrollingdatacenter

import android.content.Intent
import android.os.Bundle
import android.widget.Button
import android.widget.ImageView
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.google.android.material.timepicker.MaterialTimePicker
import com.google.android.material.timepicker.TimeFormat
import com.google.firebase.database.*

class PenjadwalanActivity : AppCompatActivity() {

    private lateinit var startTimeButton: Button
    private lateinit var endTimeButton: Button
    private lateinit var saveButton: Button
    private lateinit var recyclerView: RecyclerView
    private lateinit var deleteHistoryButton: Button


    private lateinit var database: DatabaseReference
    private lateinit var scheduleAdapter: ScheduleAdapter
    private val scheduleList = mutableListOf<Schedule>()

    private var startHour = 0
    private var startMinute = 0
    private var endHour = 0
    private var endMinute = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_penjadwalan)

        startTimeButton = findViewById(R.id.startTimeButton)
        endTimeButton = findViewById(R.id.endTimeButton)
        saveButton = findViewById(R.id.saveButton)
        recyclerView = findViewById(R.id.recyclerView)
        deleteHistoryButton = findViewById(R.id.deleteHistoryButton)
        val backButton = findViewById<ImageView>(R.id.backButton2)
        database = FirebaseDatabase.getInstance().reference

        scheduleAdapter = ScheduleAdapter(scheduleList)
        recyclerView.layoutManager = LinearLayoutManager(this)
        recyclerView.adapter = scheduleAdapter

        startTimeButton.setOnClickListener {
            showTimePicker { hour, minute ->
                startHour = hour
                startMinute = minute
                startTimeButton.text = String.format("%02d:%02d", startHour, startMinute)
            }
        }

        endTimeButton.setOnClickListener {
            showTimePicker { hour, minute ->
                endHour = hour
                endMinute = minute
                endTimeButton.text = String.format("%02d:%02d", endHour, endMinute)
            }
        }

        saveButton.setOnClickListener {
            saveScheduleToFirebase()
        }

        deleteHistoryButton.setOnClickListener {
            deleteScheduleHistory()
        }

        backButton.setOnClickListener {
            goToMainActivityBeranda()
        }

        fetchSchedulesFromFirebase()
    }

    private fun showTimePicker(onTimeSelected: (Int, Int) -> Unit) {
        val picker = MaterialTimePicker.Builder()
            .setTimeFormat(TimeFormat.CLOCK_24H)
            .setHour(12)
            .setMinute(0)
            .setTitleText("Select Time")
            .build()

        picker.addOnPositiveButtonClickListener {
            onTimeSelected(picker.hour, picker.minute)
        }

        picker.show(supportFragmentManager, "MATERIAL_TIME_PICKER")
    }

    private fun saveScheduleToFirebase() {
        val schedule = Schedule(startHour, startMinute, endHour, endMinute)

        // Save the current schedule for ESP8266
        database.child("currentSchedule").setValue(schedule)

        // Save the schedule to the history
        database.child("scheduleHistory").push().setValue(schedule)
            .addOnCompleteListener { task ->
                if (task.isSuccessful) {
                    // Data saved successfully
                    addScheduleToList(schedule)
                } else {
                    // Failed to save data
                }
            }
    }

    private fun addScheduleToList(schedule: Schedule) {
        scheduleList.add(schedule)
        scheduleAdapter.notifyItemInserted(scheduleList.size - 1)
        recyclerView.scrollToPosition(scheduleList.size - 1)
    }

    private fun fetchSchedulesFromFirebase() {
        database.child("scheduleHistory").addValueEventListener(object : ValueEventListener {
            override fun onDataChange(snapshot: DataSnapshot) {
                scheduleList.clear()
                for (scheduleSnapshot in snapshot.children) {
                    val schedule = scheduleSnapshot.getValue(Schedule::class.java)
                    schedule?.let {
                        scheduleList.add(it)
                    }
                }
                scheduleAdapter.notifyDataSetChanged()

                if (scheduleList.isNotEmpty()) {
                    recyclerView.visibility = RecyclerView.VISIBLE
                } else {
                    recyclerView.visibility = RecyclerView.GONE
                }
            }

            override fun onCancelled(error: DatabaseError) {
                // Handle possible errors.
            }
        })
    }

    private fun deleteScheduleHistory() {
        database.child("scheduleHistory").removeValue().addOnCompleteListener { task ->
            if (task.isSuccessful) {
                scheduleList.clear()
                scheduleAdapter.notifyDataSetChanged()
                recyclerView.visibility = RecyclerView.GONE
            } else {
                // Failed to delete data
            }
        }
    }

    private fun goToMainActivityBeranda() {
        val intent = Intent(this, MainActivityBeranda::class.java)
        startActivity(intent)
        finish()
    }
}