
# Skogsnet   [![Badge License]][License]

*Temperature and Humidity monitoring.*

<br>

## Concept

Using a **[I2C Grove Sensor]**, you can transmit <br>
temperature / humidity data with a **[Arduino <br>
MKR1010 Wifi][Arduino]**
board.

The Arduino sends the data through Serial USB <br>
to the host, where the program `skogsnet.cpp` <br>
reads the serial stream and parses the JSON data.

This data can then be used as inputs to some <br>
logic, a PID regulator implementation perhaps.

<br>

## Dependencies

*How to install required dependencies.*

```shell
sudo pacman -S nlohmann-json
```

<br>
<br>

## Building

*How to compile the project code.*

```shell
make
```

<br>
<br>

## Running

*How to start the program.*

```shell
make run
```

<br>


<!----------------------------------------------------------------------------->

[Badge License]: https://img.shields.io/badge/License-GPL_3-blue.svg?style=for-the-badge

[I2C Grove Sensor]: https://www.seeedstudio.com/Grove-Temperature-Humidity-Sensor-High-Accuracy-Mini.html
[Arduino]: https://docs.arduino.cc/hardware/mkr-1000-wifi

[License]: #

