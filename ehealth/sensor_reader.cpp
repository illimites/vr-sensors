#include <eHealth.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <time.h>
#include <cassert>

#include "measurement.h"

using namespace std;


struct Settings {
    int    measurement_interval;
    string log_path;
};


void print_digits(ostream & output, const int * digits, int num_digits) {
    assert(num_digits >= 0);

    for (int i = num_digits - 1; i >= 0; --i) {
        assert(0 <= digits[i] <= 9 || digits[i] == -1);

        if (digits[i] == -1)
            output << "?";
        else
            output << digits[i];
    }
}


void print_readings(const SensorReadings & readings) {
    cout << "system        | timestamp                = "    << readings.timestamp.tv_sec << "." << setfill('0') << setw(9) << readings.timestamp.tv_nsec << " ns" << endl;
    cout << "pulsioximeter | PRbpm                    = "    << readings.bpm << " (";
    print_digits(cout, readings.bpm_digits, 3);
    cout << ")" << endl;
    cout << "ECG           | ECG                      = "    << readings.ecg << endl;
    cout << "galvanic      | Skin conductance voltage = "    << readings.conductance_voltage << " V" << endl;
    cout << "==============================================" << endl;
}


void log_header(ofstream & log) {
    log << "timestamp"       << ", ";
    log << "PRbpm"           << ", ";
    log << "PRbpm digit 100" << ", ";
    log << "PRbpm digit 10"  << ", ";
    log << "PRbpm digit 1"   << ", ";
    log << "ECG"             << ", ";
    log << "Skin conductance voltage";
    log << endl;
}


void log_readings(ofstream & log, const SensorReadings & readings) {
    log << readings.timestamp.tv_sec << "." << setfill('0') << setw(9) << readings.timestamp.tv_nsec << ", ";
    log << readings.bpm               << ", ";
    log << readings.bpm_digits[2]     << ", ";
    log << readings.bpm_digits[1]     << ", ";
    log << readings.bpm_digits[0]     << ", ";
    log << readings.ecg               << ", ";
    log << readings.conductance_voltage;
    log << endl;
}


bool process_command_line(int argc, char *argv[], Settings & settings) {
    settings.log_path             = "";
    settings.measurement_interval = 0;

    if (argc > 1)
        settings.log_path = argv[1];

    if (argc > 2) {
        istringstream interval_string(argv[2]);
        int           parsed_interval;

        if (!(interval_string >> parsed_interval)) {
            cerr << "Invalid argument: " << argv[2] << ". Expected an integer." << endl;
            return false;
        }

        settings.measurement_interval = parsed_interval;
    }

    if (settings.measurement_interval < 0) {
        cerr << "Measurement interval must be positive" << endl;
        return false;
    }

    if (settings.measurement_interval > 0)
        cout << "Measurement interval: " << settings.measurement_interval << " ms" << endl;
    else
        cout << "Measurement interval: no waiting between measurements" << endl;

    if (settings.log_path != "")
        cout << "Log path:             " << settings.log_path << endl;
    else
        cout << "Logging disabled" << endl;

    return true;
}


int main (int argc, char *argv[]){
    setup_sensors();

    Settings settings;
    if (!process_command_line(argc, argv, settings))
        return 1;

    ofstream log_file;
    if (settings.log_path != "")
    {
        log_file.open(settings.log_path.c_str());
        log_header(log_file);
    }

    // nanosleep() could be off by somewhere around 10-100 microseconds due to system overhead so there's no reason to sleep less than that.
    // We need microsecond accuracy so sleeping 1/10 of a microsecond seems about right.
    timespec nanosleep_interval;
    nanosleep_interval.tv_sec  = 0;
    nanosleep_interval.tv_nsec = 100000;

    do {
        timespec last_measurement_time;
        clock_gettime(CLOCK_MONOTONIC, &last_measurement_time);

        SensorReadings readings = read_sensors();

        // Print and log stuff. If log file name has not been specified, printing to the screen is enough.
        print_readings(readings);
        if (settings.log_path != "")
            log_readings(log_file, readings);

        // Sleep in short intervals until the required measurement_interval is reached.
        // But don't sleep if not necessary. In particular the interval can be zero.
        timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        while ((current_time.tv_sec - last_measurement_time.tv_sec) * 1000 + (current_time.tv_nsec - last_measurement_time.tv_nsec) / 1000000 < settings.measurement_interval) {
            nanosleep(&nanosleep_interval, NULL);
            clock_gettime(CLOCK_MONOTONIC, &current_time);
        }
    } while (true);

    if (settings.log_path != "")
        log_file.close();

    return 0;
}
