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


## Dependencies

_How to install required dependencies._

```shell
sudo pacman -S raylib nlohmann-json base-devel cmake 
```

## Running

_How to start both programs._

```shell
make -j 2 skogsnet raylibfrontend
```

## Skogsnet output
```bash
> make skogsnet_v0.0.4                                     %

Building the program
clang -Wall -g -std=c++11  -I/usr/include -o ./build/skogsnet_v0.0.4 ./code/PID.cpp ./code/skogsnet.cpp -L /usr/lib -lstdc++ -lm
./build/skogsnet_v0.0.4
	Setting up time measurement and serial communications...
./build/skogsnet_v0.0.4

      Trying port: /dev/ttyACM0

        Skogsnet is running now, connected to port: /dev/ttyACM0


        Measurement:
        Timestamp:		1724430827844
        TemperatureCelcius:	23.250000
        Humidity:		83.937500

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 23.250000	pid.out: -10.000000	correctedOutput: 0.000000

	Successfully wrote to file: output.dat

	Simulated 1000 steps out of 1000 total steps
	PID computation total run time: (s): 0.000101
	PID computation total run time (ms): 0.100851
	PID computation total run time (μS): 100.851059
----------------------------------------------------------

        Measurement:
        Timestamp:		1724430832844
        TemperatureCelcius:	23.250000
        Humidity:		83.812500

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 23.250000	pid.out: -10.000000	correctedOutput: -9.247562

	Successfully wrote to file: output.dat

	Simulated 1000 steps out of 1000 total steps
	PID computation total run time: (s): 0.000175
	PID computation total run time (ms): 0.174999
	PID computation total run time (μS): 174.999237
----------------------------------------------------------

        Measurement:
        Timestamp:		1724430837852
        TemperatureCelcius:	23.250000
        Humidity:		83.875000

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 23.250000	pid.out: -10.000000	correctedOutput: -18.974636

	Successfully wrote to file: output.dat

	Simulated 1000 steps out of 1000 total steps
	PID computation total run time: (s): 0.000125
	PID computation total run time (ms): 0.124931
	PID computation total run time (μS): 124.931335
----------------------------------------------------------

        Measurement:
        Timestamp:		1724430842916
        TemperatureCelcius:	23.218750
        Humidity:		83.875000

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 23.218750	pid.out: -10.000000	correctedOutput: -28.508919

	Successfully wrote to file: output.dat

	Simulated 1000 steps out of 1000 total steps
	PID computation total run time: (s): 0.000164
	PID computation total run time (ms): 0.163794
	PID computation total run time (μS): 163.793564
----------------------------------------------------------
^C
        dummy: 2
        Exiting successfully.

        [Performance Metrics]
        Total memory allocated 		 = 0.0 MB
        Current Wall clock run time 	 = 16.7 secs
```

## Frontend

![screenshot.png](screenshot.png "Frontend")

<!----------------------------------------------------------------------------->

[badge license]: https://img.shields.io/badge/License-GPL_3-blue.svg?style=for-the-badge
[i2c grove sensor]: https://www.seeedstudio.com/Grove-Temperature-Humidity-Sensor-High-Accuracy-Mini.html
[arduino]: https://docs.arduino.cc/hardware/mkr-1000-wifi
[license]: #
