#include <arduPi.h>
#include <eHealth.h>
#include <cassert>

#include "measurement.h"

static int pulsioximeter_bpm_digits[3];
static int pulsioximeter_oxygen_saturation_digits[2];

int digits_to_int(int hundreds, int tens, int units);


SensorReadings read_sensors() {
    SensorReadings readings;

    assert(0 <= pulsioximeter_bpm_digits[2] <= 9 || pulsioximeter_bpm_digits[2] == -1);
    assert(0 <= pulsioximeter_bpm_digits[1] <= 9 || pulsioximeter_bpm_digits[1] == -1);
    assert(0 <= pulsioximeter_bpm_digits[0] <= 9 || pulsioximeter_bpm_digits[0] == -1);

    clock_gettime(CLOCK_MONOTONIC, &readings.timestamp);
    readings.bpm                 = digits_to_int(pulsioximeter_bpm_digits[2], pulsioximeter_bpm_digits[1], pulsioximeter_bpm_digits[0]);
    readings.bpm_digits[2]       = pulsioximeter_bpm_digits[2];
    readings.bpm_digits[1]       = pulsioximeter_bpm_digits[1];
    readings.bpm_digits[0]       = pulsioximeter_bpm_digits[0];
    readings.ecg                 = get_ecg();
    readings.conductance_voltage = get_skin_conductance_voltage();

    return readings;
}


void pulsioximeter_interrupt_handler() {
    static int run_count = 0;

    ++run_count;

    // Read only once per 50 calls to reduce the latency.
    // That's what eHealth examples do.
    if (run_count == 50) {
        update_pulsioximeter_globals();
        run_count = 0;
    }
}


void setup_sensors() {
    eHealth.initPulsioximeter();
    attachInterrupt(6, pulsioximeter_interrupt_handler, RISING);
}


int8_t segToNumber(uint8_t A, uint8_t B, uint8_t C, uint8_t D, uint8_t E, uint8_t F, uint8_t G)
{
    // This function is based on eHealthClass::segToNumber from eHealth.cpp.

    if        ((A == 1) && (B == 1) && (C == 1) && (D == 0) && (E == 1) && (F == 1) && (G == 1)) {
        return 0;
    } else if ((A == 0) && (B == 1) && (C == 0) && (D == 0) && (E == 1) && (F == 0) && (G == 0)) {
        return 1;
    } else if ((A == 1) && (B == 1) && (C == 0) && (D == 1) && (E == 0) && (F == 1) && (G == 1)) {
        return 2;
    } else if ((A == 1) && (B == 1) && (C == 0) && (D == 1) && (E == 1) && (F == 0) && (G == 1)) {
        return 3;
    } else if ((A == 0) && (B == 1) && (C == 1) && (D == 1) && (E == 1) && (F == 0) && (G == 0)) {
        return 4;
    } else if ((A == 1) && (B == 0) && (C == 1) && (D == 1) && (E == 1) && (F == 0) && (G == 1)) {
        return 5;
    } else if ((A == 1) && (B == 0) && (C == 1) && (D == 1) && (E == 1) && (F == 1) && (G == 1)) {
        return 6;
    } else if ((A == 1) && (B == 1) && (C == 0) && (D == 0) && (E == 1) && (F == 0) && (G == 0)) {
        return 7;
    } else if ((A == 1) && (B == 1) && (C == 1) && (D == 1) && (E == 1) && (F == 1) && (G == 1)) {
        return 8;
    } else if ((A == 1) && (B == 1) && (C == 1) && (D == 1) && (E == 1) && (F == 0) && (G == 1)) {
        return 9;
    } else  {
        return -1;
    }
}


int digits_to_int(int hundreds, int tens, int units) {
    assert(0 <= hundreds < 10 || hundreds == -1);
    assert(0 <= tens     < 10 || tens     == -1);
    assert(0 <= units    < 10 || units    == -1);

    // If any of the digits could not be properly decoded, return -1.
    // Don't do anything stupid like assuming that the digit was zero. Don't make things up.
    if (hundreds == -1 || tens == -1 || units == -1)
        return -1;

    return hundreds * 100 + tens * 10 + units;
}


void update_pulsioximeter_globals()
{
    // This function is based on eHealthClass::readPulsioximeter() from eHealth.cpp.

    int8_t  digito[200];
    uint8_t A = 0;
    uint8_t B = 0;
    uint8_t C = 0;
    uint8_t D = 0;
    uint8_t E = 0;
    uint8_t F = 0;
    uint8_t G = 0;

    // Read LED all segments of pulsioximeter display.
    // What a roundabout way to communicate...
    for (int i = 0; i < 199 ; i++) {
        A = !digitalRead(13);
        B = !digitalRead(12);
        C = !digitalRead(11);
        D = !digitalRead(10);
        E = !digitalRead(9);
        F = !digitalRead(8);
        G = !digitalRead(7);

        digito[i] = segToNumber(A, B, C, D, E, F, G);

        delayMicroseconds(43);
    }

    // NOTE: A digit being zero in the original code could mean either that it was actually zero
    // or that it was just malformed (the code did not discern between these two).
    // I'm assuming that the comparisons with zero are being done to avoid those malformed digits so
    // I'm now comparing with -1 instead (that's what is returned now when the digit is malformed).
    if (digito[142] != -1 && digito[181] == -1) {
        pulsioximeter_oxygen_saturation_digits[1] = digito[142];
        pulsioximeter_oxygen_saturation_digits[0] = digito[59];
        pulsioximeter_bpm_digits[2]               = digito[137];
        pulsioximeter_bpm_digits[1]               = digito[10];
        pulsioximeter_bpm_digits[0]               = digito[2];
    }
    else if (digito[136] != -1 && digito[62] == -1) {
        pulsioximeter_oxygen_saturation_digits[1] = digito[179];
        pulsioximeter_oxygen_saturation_digits[0] = digito[136];
        pulsioximeter_bpm_digits[2]               = digito[127];
        pulsioximeter_bpm_digits[1]               = digito[10];
        pulsioximeter_bpm_digits[0]               = digito[2];
    }
    else if (digito[145] != -1 && digito[62] == -1) {
        pulsioximeter_oxygen_saturation_digits[1] = digito[181];
        pulsioximeter_oxygen_saturation_digits[0] = digito[142];
        pulsioximeter_bpm_digits[2]               = digito[50];
        pulsioximeter_bpm_digits[1]               = digito[10];
        pulsioximeter_bpm_digits[0]               = digito[2];
    }
    else if (digito[53] != -1 && digito[62] == -1) {
        pulsioximeter_oxygen_saturation_digits[1] = digito[179];
        pulsioximeter_oxygen_saturation_digits[0] = digito[53];
        pulsioximeter_bpm_digits[2]               = digito[41];
        pulsioximeter_bpm_digits[1]               = digito[10];
        pulsioximeter_bpm_digits[0]               = digito[2];
    }
    else if (digito[174] != -1 && digito[181] == -1) {
        pulsioximeter_oxygen_saturation_digits[1] = digito[179];
        pulsioximeter_oxygen_saturation_digits[0] = digito[59];
        pulsioximeter_bpm_digits[2]               = digito[50];
        pulsioximeter_bpm_digits[1]               = digito[10];
        pulsioximeter_bpm_digits[0]               = digito[2];
    }
    else {
        pulsioximeter_oxygen_saturation_digits[1] = digito[179];
        pulsioximeter_oxygen_saturation_digits[0] = digito[59];
        pulsioximeter_bpm_digits[2]               = digito[50];
        pulsioximeter_bpm_digits[1]               = digito[10];
        pulsioximeter_bpm_digits[0]               = digito[2];
    }
}


float get_ecg()
{
    // This function is based on eHealthClass::getECG() from eHealth.cpp.

    // binary to voltage conversion
    return (float)analogRead(0) * 5 / 1023.0;
}


float get_skin_conductance_voltage()
{
    // This function is based on eHealthClass::getSkinConductance() from eHealth.cpp.

    // getSkinConductance() has delays of 2 ms both before and after reading the analog input.
    // And no comments saying why it's waiting. If they were somewhere in between calls I'd say that
    // we have to wait to get the data ready or something. But there was delay before the function
    // even does anything! What was the point? We already have the delay in the main loop. Pointless
    // delays here don't let us do other measurements in parallel.

    // Read from analog pin 2 and convert to voltage
    int   sensorValue = analogRead(2);
    float voltage     = (sensorValue * 5.0) / 1023;

    return voltage;
}
