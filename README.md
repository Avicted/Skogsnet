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

<br>

## Example Output

```bash
./build/skogsnet_v0.0.1
        Settings up time measurement and serial communications...
./build/skogsnet_v0.0.1

      Trying port: /dev/ttyACM0

        Skogsnet is running now, connected to port: /dev/ttyACM0

        buffer: {"temperature_celcius":28.125,"humidity":63.5625}

{
    "humidity": 63.5625,
    "temperature_celcius": 28.125
}


        Measurement:
        Timestamp:		02.07.2022 13:14:11
        TemperatureCelcius:	28.125000
        Humidity:		63.562500
Time (s)	System Output	ControllerOutput
t: 0.999999	measurement: 28.125000	pid.out: -10.000000
        buffer: {"temperature_celcius":28.125,"humidity":63.625}

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
