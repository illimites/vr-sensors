# VR sensors

## e-Health Sensor Shield measurement reader

### Prerequisites

The program is meant to run under Linux on Raspberry Pi with [e-Health Sensor Shield v2.0](https://www.cooking-hacks.com/documentation/tutorials/ehealth-biometric-sensor-platform-arduino-raspberry-pi-medical) attached.

### How to use

First, download the code and build `sensor_reader`
``` bash
git clone https://github.com/illimites/vr-sensors
cd vr-sensors/ehealth
make
```

Now it's ready to run.

``` bash
sudo bin/sensor_reader <CSV log path> <measurement interval>

```

The program accepts two arguments: the path to the .csv log file and the interval between measurements.
Both are optional.
If the path is omitted, the measurements are not written to a file (but they're still printed to the console).
Measurement interval is the delay between subsequent measurements in milliseconds.
If not specified, zero is assumed and the measurements are performed in a loop without any extra delay beyond what's needed to properly communicate with the device.

The program needs to be able to communicate with the device via `/dev/mem`.
To achieve this you can simply run it as root.

Example:

``` bash
sudo bin/sensor_reader /tmp/john-smith-2016-06-05.csv 10

```

### eHealth examples

You can build the examples included with the [eHealth](https://www.cooking-hacks.com/documentation/tutorials/ehealth-biometric-sensor-platform-arduino-raspberry-pi-medical#step3_2) library by running

``` bash
make examples
```

Built executables will be placed under `bin/`.

The example programs are not needed by `sensor_reader` to run but they're convenient when you want to make sure that the device works properly.
