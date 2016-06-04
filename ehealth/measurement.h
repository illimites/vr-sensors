#ifndef __MEASUREMENT_H_INCLUDED__
#define __MEASUREMENT_H_INCLUDED__

struct SensorReadings {
    timespec timestamp;
    int      bpm;
    int      bpm_digits[3];
    float    ecg;
    float    conductance_voltage;
};

SensorReadings read_sensors();
void           setup_sensors();

void  update_pulsioximeter_globals();
float get_ecg();
float get_skin_conductance_voltage();

#endif // __MEASUREMENT_H_INCLUDED__
