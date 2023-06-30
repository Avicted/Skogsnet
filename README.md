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
Creating directories
mkdir -p build

Building and running the raylib frontend
./raylib_frontend/build.sh&

Building the program
clang -Wall -g -std=c++11 -fopenmp -I/usr/include -o ./build/skogsnet_v0.0.1 ./code/PID.cpp ./code/skogsnet.cpp -L /usr/lib -fopenmp -lstdc++ -lm
Compiling raylib_frontend

-- Testing if -Werror=pointer-arith can be used -- compiles
-- Testing if -Werror=implicit-function-declaration can be used -- compiles
-- Testing if -fno-strict-aliasing can be used -- compiles
-- Using raylib's GLFW
-- Including X11 support
-- Audio Backend: miniaudio
-- Building raylib static library
-- Generated build type: Debug
-- Compiling with the flags:
--   PLATFORM=PLATFORM_DESKTOP
--   GRAPHICS=GRAPHICS_API_OPENGL_33
-- Configuring done (0.3s)
-- Generating done (0.0s)
-- Build files have been written to: /home/avic/projects/Skogsnet/build
make[1]: Entering directory '/home/avic/projects/Skogsnet/build'
[ 70%] Built target glfw
[ 94%] Built target raylib
[ 97%] Building CXX object CMakeFiles/Frontend.dir/Frontend.cpp.o
[100%] Linking CXX executable Frontend
[100%] Built target Frontend
make[1]: Leaving directory '/home/avic/projects/Skogsnet/build'
make[1]: Entering directory '/home/avic/projects/Skogsnet'
make[1]: *** ../build: No such file or directory.  Stop.
make[1]: Leaving directory '/home/avic/projects/Skogsnet'
Running raylib_frontend

	No input args OK!
Current working directory: /home/avic/projects/Skogsnet
	Hello from raylib_frontend!
	 Successfully read 121 data points from file!
./build/skogsnet_v0.0.1
        Settings up time measurement and serial communications...
./build/skogsnet_v0.0.1

      Trying port: /dev/ttyACM0

        Skogsnet is running now, connected to port: /dev/ttyACM0

{
    "humidity": 65.6875,
    "temperature_celcius": 25.25
}


        Measurement:
        Timestamp:		1688134244402
        TemperatureCelcius:	25.250000
        Humidity:		65.687500

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 25.250000	pid.out: -10.000000	correctedOutput: 0.000000
        Successfully wrote to file: 1688134244402	25.250000	65.687500	-10.000000	0.000000

        Simulated 1001 steps out of 1000 total steps
        PID computation run time = 0.0 secs out of our time budget: 1.000000
	 Successfully read latest data point from file at time: 1688134244402
	 DataPointCount: 122
{
    "humidity": 65.8125,
    "temperature_celcius": 25.25
}


        Measurement:
        Timestamp:		1688134249464
        TemperatureCelcius:	25.250000
        Humidity:		65.812500

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 25.250000	pid.out: -10.000000	correctedOutput: -9.308496
        Successfully wrote to file: 1688134249464	25.250000	65.812500	-10.000000	-9.308496

        Simulated 1001 steps out of 1000 total steps
        PID computation run time = 0.0 secs out of our time budget: 1.000000
	 Successfully read latest data point from file at time: 1688134249464
	 DataPointCount: 123
{
    "humidity": 65.6875,
    "temperature_celcius": 25.25
}

```

## Dependencies

_How to install required dependencies._

```shell
sudo pacman -S nlohmann-json
```

## Running

_How to start both programs._

```shell
make
```

_How to start only the logging program._

```shell
make run
```

_How to start only the Visualization program._

```shell
make gui
```

<br>

![screenshot.png](screenshot.png "screenshot.png")

<!----------------------------------------------------------------------------->

[badge license]: https://img.shields.io/badge/License-GPL_3-blue.svg?style=for-the-badge
[i2c grove sensor]: https://www.seeedstudio.com/Grove-Temperature-Humidity-Sensor-High-Accuracy-Mini.html
[arduino]: https://docs.arduino.cc/hardware/mkr-1000-wifi
[license]: #
