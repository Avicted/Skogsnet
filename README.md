# Temperature and Humidity monitoring.

Using an **[Arduino MKR1010 Wifi](https://docs.arduino.cc/hardware/mkr-1000-wifi)** with an **[I2C Grove Sensor](https://www.seeedstudio.com/Grove-Temperature-Humidity-Sensor-High-Accuracy-Mini.html)** for temp and humidity.
The arduino sends the data through Serial USB to the host, where the program **skogsnet.cpp** reads the serial stream and parses the JSON data. This data
can then be used as inputs to some logic, a PID regulator implementation perhaps.

```bash
# Dependencies
sudo pacman -S nlohmann-json

# Build
make

# Run
make run
```
