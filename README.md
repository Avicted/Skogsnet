# Skogsnet   [![Badge License]][license]

_Temperature and Humidity monitoring._

<br>

## Concept

Using a **[I2C Grove Sensor]**, you can transmit <br>
temperature / humidity data with a **[Arduino <br>
MKR1010 Wifi][arduino]**
board.

The Arduino sends the data through Serial USB <br>
to the host, where the program `skogsnet.cpp` <br>
reads the serial stream and parses the JSON data.

This data can then be used as inputs to some <br>
logic, a PID regulator implementation perhaps.

The data received by the skogsnet program is written out to **output.dat**

<br>

## Example make Output

```bash
Cleaning
rm -r build 2> /dev/null || true
rm -r code/*.o 2> /dev/null || true

Taking a backup of the old results
cp output.dat output_old.dat 2> /dev/null || true
rm output.dat 2> /dev/null || true

Creating directories
mkdir -p build
g++  -std=c++23 -g -Ofast -Wall -I/usr/include  -c -o code/PID.o code/PID.cpp
g++  -std=c++23 -g -Ofast -Wall -I/usr/include  -c -o code/skogsnet.o code/skogsnet.cpp

Building the program
g++ -std=c++23 -g -Ofast -Wall -I/usr/include -o ./build/skogsnet_v0.0.1 ./code/PID.cpp ./code/skogsnet.cpp -L /usr/lib -lm -fopenmp
./build/skogsnet_v0.0.1
        Settings up time measurement and serial communications...
./build/skogsnet_v0.0.1

      Trying port: /dev/ttyACM0

        Skogsnet is running now, connected to port: /dev/ttyACM0

{
    "humidity": 56.6875,
    "temperature_celcius": 27.75
}


        Measurement:
        Timestamp:              1656855327318
        TemperatureCelcius:     27.750000
        Humidity:               56.687500

        Time (s)        System Output           ControllerOutput        CorrectedOutput
        t: 0.000000     measurement: 27.750000  pid.out: -10.000000     correctedOutput: 0.000000

        Simulated 1001 steps out of 1000 total steps
        PID computation run time = 0.0 secs out of our time budget: 1.000000

```

## Dependencies

_How to install required dependencies._

```shell
sudo pacman -S nlohmann-json
```

<br>
<br>

## Building

_How to compile the project code._

```shell
make
```

<br>
<br>

## Running

_How to start the program._

```shell
make run
```

<br>

<!----------------------------------------------------------------------------->

[badge license]: https://img.shields.io/badge/License-GPL_3-blue.svg?style=for-the-badge
[i2c grove sensor]: https://www.seeedstudio.com/Grove-Temperature-Humidity-Sensor-High-Accuracy-Mini.html
[arduino]: https://docs.arduino.cc/hardware/mkr-1000-wifi
[license]: #
