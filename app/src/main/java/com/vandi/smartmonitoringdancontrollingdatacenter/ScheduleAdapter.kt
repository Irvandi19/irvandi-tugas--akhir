package com.vandi.smartmonitoringdancontrollingdatacenter

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView

class ScheduleAdapter(private val scheduleList: List<Schedule>) : RecyclerView.Adapter<ScheduleAdapter.ScheduleViewHolder>() {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ScheduleViewHolder {
        val itemView = LayoutInflater.from(parent.context).inflate(R.layout.item_schedule, parent, false)
        return ScheduleViewHolder(itemView)
    }

    override fun onBindViewHolder(holder: ScheduleViewHolder, position: Int) {
        val schedule = scheduleList[position]
        holder.startTimeTextView.text = String.format("%02d:%02d", schedule.startHour, schedule.startMinute)
        holder.endTimeTextView.text = String.format("%02d:%02d", schedule.endHour, schedule.endMinute)
    }

    override fun getItemCount() = scheduleList.size

    class ScheduleViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        val startTimeTextView: TextView = itemView.findViewById(R.id.startTimeTextView)
        val endTimeTextView: TextView = itemView.findViewById(R.id.endTimeTextView)
    }
}