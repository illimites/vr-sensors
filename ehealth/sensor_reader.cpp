#include <eHealth.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <sys/time.h>

using namespace std;


struct SensorReadings {
    timespec timestamp;
    int      bpm;
    int      oxygen_saturation;
    float    ecg;
    float    conductance;
    float    resistance;
    float    conductance_voltage;
};


struct Settings {
    int    measurement_interval;
    string log_path;
};


void pulsioximeter_interrupt_handler() {
    static int run_count = 0;

    ++run_count;

    // Read only once per 50 calls to reduce the latency.
    // That's what eHealth examples do.
    if (run_count == 50) {
        eHealth.readPulsioximeter();
        run_count = 0;
    }
}


void setup_sensors() {
    eHealth.initPulsioximeter();
    attachInterrupt(6, pulsioximeter_interrupt_handler, RISING);
}


SensorReadings read_sensors() {
    SensorReadings readings;

    clock_gettime(CLOCK_MONOTONIC, &readings.timestamp);
    readings.bpm                 = eHealth.getBPM();
    readings.oxygen_saturation   = eHealth.getOxygenSaturation();
    readings.ecg                 = eHealth.getECG();
    readings.conductance         = eHealth.getSkinConductance();
    readings.resistance          = eHealth.getSkinResistance();
    readings.conductance_voltage = eHealth.getSkinConductanceVoltage();

    return readings;
}


void print_readings(const SensorReadings & readings) {
    cout << "system        | timestamp                = "    << readings.timestamp.tv_sec << "." << setfill('0') << setw(9) << readings.timestamp.tv_nsec << " ns" << endl;
    cout << "pulsioximeter | PRbpm                    = "    << readings.bpm << endl;
    cout << "pulsioximeter | Oxygen saturation (SPo2) = "    << readings.oxygen_saturation << endl;
    cout << "ECG           | ECG                      = "    << readings.ecg << endl;
    cout << "galvanic      | Skin conductance         = "    << readings.conductance << endl;
    cout << "galvanic      | Skin resistance          = "    << readings.resistance << endl;
    cout << "galvanic      | Skin conductance voltage = "    << readings.conductance_voltage << " V" << endl;
    cout << "==============================================" << endl;
}


void log_header(ofstream & log) {
    log << "timestamp"                 << ", ";
    log << "PRbpm"                     << ", ";
    log << "Oxygen saturation (SPo2)"  << ", ";
    log << "ECG"                       << ", ";
    log << "Skin conductance"          << ", ";
    log << "Skin resistance"           << ", ";
    log << "Skin conductance voltage";
    log << endl;
}


void log_readings(ofstream & log, const SensorReadings & readings) {
    log << readings.timestamp.tv_sec << "." << setfill('0') << setw(9) << readings.timestamp.tv_nsec << ", ";
    log << readings.bpm               << ", ";
    log << readings.oxygen_saturation << ", ";
    log << readings.ecg               << ", ";
    log << readings.conductance       << ", ";
    log << readings.resistance        << ", ";
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

    do {
        SensorReadings readings = read_sensors();

        // Print and log stuff. If log file name has not been specified, printing to the screen is enough.
        print_readings(readings);
        if (settings.log_path != "")
            log_readings(log_file, readings);
        delay(settings.measurement_interval);
    } while (true);

    if (settings.log_path != "")
        log_file.close();

    return 0;
}

