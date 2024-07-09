package com.vandi.smartmonitoringdancontrollingdatacenter

data class Schedule(
    val startHour: Int = 0,
    val startMinute: Int = 0,
    val endHour: Int = 0,
    val endMinute: Int = 0
)