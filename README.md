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
~/projects/Skogsnet refactor-cleanup > make      

Creating directories
mkdir -p build

Building and running the raylib frontend
./raylib_frontend/build.sh&
g++  -Wall -g -std=c++11  -I/usr/include  -c -o code/PID.o code/PID.cpp
Compiling raylib_frontend

g++  -Wall -g -std=c++11  -I/usr/include  -c -o code/skogsnet.o code/skogsnet.cpp
-- The CXX compiler identification is GNU 13.1.1
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done

Building the program
clang -Wall -g -std=c++11  -I/usr/include -o ./build/skogsnet_v0.0.3 ./code/PID.cpp ./code/skogsnet.cpp -L /usr/lib -fopenmp -lstdc++ -lm
./build/skogsnet_v0.0.3
	Settings up time measurement and serial communications...
./build/skogsnet_v0.0.3

      Trying port: /dev/ttyACM0

        Skogsnet is running now, connected to port: /dev/ttyACM0


        Measurement:
        Timestamp:		1688231737243
        TemperatureCelcius:	24.093750
        Humidity:		65.937500

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 24.093750	pid.out: -10.000000	correctedOutput: 0.000000

	Successfully wrote to file: output.dat

	Simulated 1000 steps out of 1000 total steps
	PID computation total run time: (s): 0.000623
	PID computation total run time (ms): 0.622988
	PID computation total run time (μS): 622.987747
----------------------------------------------------------

        Measurement:
        Timestamp:		1688231742303
        TemperatureCelcius:	24.031250
        Humidity:		65.937500

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 24.031250	pid.out: -10.000000	correctedOutput: -9.278154

	Successfully wrote to file: output.dat

	Simulated 1000 steps out of 1000 total steps
	PID computation total run time: (s): 0.000165
	PID computation total run time (ms): 0.164986
	PID computation total run time (μS): 164.985657
----------------------------------------------------------
-- The C compiler identification is GNU 13.1.1
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Performing Test COMPILER_HAS_THOSE_TOGGLES
-- Performing Test COMPILER_HAS_THOSE_TOGGLES - Success
-- Testing if -Werror=pointer-arith can be used -- compiles
-- Testing if -Werror=implicit-function-declaration can be used -- compiles
-- Testing if -fno-strict-aliasing can be used -- compiles
-- Using raylib's GLFW
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE  
-- Including X11 support

        Measurement:
        Timestamp:		1688231752425
        TemperatureCelcius:	24.031250
        Humidity:		65.937500

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 24.031250	pid.out: -10.000000	correctedOutput: -28.538321

	Successfully wrote to file: output.dat

	Simulated 1000 steps out of 1000 total steps
	PID computation total run time: (s): 0.000128
	PID computation total run time (ms): 0.128031
	PID computation total run time (μS): 128.030777
----------------------------------------------------------
-- Found X11: /usr/include   
-- Looking for XOpenDisplay in /usr/lib/libX11.so;/usr/lib/libXext.so
-- Looking for XOpenDisplay in /usr/lib/libX11.so;/usr/lib/libXext.so - found
-- Looking for gethostbyname
-- Looking for gethostbyname - found
-- Looking for connect
-- Looking for connect - found
-- Looking for remove
-- Looking for remove - found
-- Looking for shmat
-- Looking for shmat - found
-- Looking for IceConnectionNumber in ICE
-- Looking for IceConnectionNumber in ICE - found
-- Audio Backend: miniaudio
-- Building raylib static library
-- Generated build type: Debug
-- Compiling with the flags:
--   PLATFORM=PLATFORM_DESKTOP
--   GRAPHICS=GRAPHICS_API_OPENGL_33
-- Configuring done (22.8s)
-- Generating done (0.0s)
-- Build files have been written to: /home/avic/projects/Skogsnet/build
make[1]: Entering directory '/home/avic/projects/Skogsnet/build'
[  2%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/context.c.o
[  5%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/init.c.o
[  8%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/input.c.o
[ 11%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/monitor.c.o
[ 14%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/platform.c.o
[ 17%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/vulkan.c.o
[ 20%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/window.c.o
[ 23%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/egl_context.c.o
[ 26%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/osmesa_context.c.o
[ 29%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/null_init.c.o
[ 32%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/null_monitor.c.o
[ 35%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/null_window.c.o
[ 38%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/null_joystick.c.o
[ 41%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/posix_module.c.o
[ 44%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/posix_time.c.o
[ 47%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/posix_thread.c.o
[ 50%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/x11_init.c.o
[ 52%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/x11_monitor.c.o
[ 55%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/x11_window.c.o
[ 58%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/xkb_unicode.c.o
[ 61%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/glx_context.c.o
[ 64%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/linux_joystick.c.o
[ 67%] Building C object _deps/raylib-build/raylib/external/glfw/src/CMakeFiles/glfw.dir/posix_poll.c.o
[ 70%] Linking C static library libglfw3.a
[ 70%] Built target glfw
[ 73%] Building C object _deps/raylib-build/raylib/CMakeFiles/raylib.dir/rcore.c.o
[ 76%] Building C object _deps/raylib-build/raylib/CMakeFiles/raylib.dir/rmodels.c.o
[ 79%] Building C object _deps/raylib-build/raylib/CMakeFiles/raylib.dir/rshapes.c.o
[ 82%] Building C object _deps/raylib-build/raylib/CMakeFiles/raylib.dir/rtext.c.o
[ 85%] Building C object _deps/raylib-build/raylib/CMakeFiles/raylib.dir/rtextures.c.o

        Measurement:
        Timestamp:		1688231757486
        TemperatureCelcius:	24.062500
        Humidity:		66.000000

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 24.062500	pid.out: -10.000000	correctedOutput: -37.883041

	Successfully wrote to file: output.dat

	Simulated 1000 steps out of 1000 total steps
	PID computation total run time: (s): 0.000080
	PID computation total run time (ms): 0.080109
	PID computation total run time (μS): 80.108643
----------------------------------------------------------
[ 88%] Building C object _deps/raylib-build/raylib/CMakeFiles/raylib.dir/utils.c.o
[ 91%] Building C object _deps/raylib-build/raylib/CMakeFiles/raylib.dir/raudio.c.o
[ 94%] Linking C static library libraylib.a
[ 94%] Built target raylib
[ 97%] Building CXX object CMakeFiles/Frontend.dir/Frontend.cpp.o
[100%] Linking CXX executable Frontend

        Measurement:
        Timestamp:		1688231762547
        TemperatureCelcius:	24.062500
        Humidity:		65.937500

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 24.062500	pid.out: -10.000000	correctedOutput: -47.042549

	Successfully wrote to file: output.dat

	Simulated 1000 steps out of 1000 total steps
	PID computation total run time: (s): 0.000147
	PID computation total run time (ms): 0.147104
	PID computation total run time (μS): 147.104263
----------------------------------------------------------
[100%] Built target Frontend
make[1]: Leaving directory '/home/avic/projects/Skogsnet/build'
make[1]: Entering directory '/home/avic/projects/Skogsnet/build'
[ 70%] Built target glfw
[ 94%] Built target raylib
[100%] Built target Frontend
make[1]: Leaving directory '/home/avic/projects/Skogsnet/build'
Running raylib_frontend

	No input args OK!
Current working directory: /home/avic/projects/Skogsnet
	Hello from raylib_frontend!
	 Successfully read 11002 data points from file!

        Measurement:
        Timestamp:		1688231767608
        TemperatureCelcius:	24.062500
        Humidity:		65.937500

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 24.062500	pid.out: -10.000000	correctedOutput: -56.020504

	Successfully wrote to file: output.dat

	Simulated 1000 steps out of 1000 total steps
	PID computation total run time: (s): 0.000196
	PID computation total run time (ms): 0.195980
	PID computation total run time (μS): 195.980072
----------------------------------------------------------
	DataPointCount: 11003

        Measurement:
        Timestamp:		1688231772669
        TemperatureCelcius:	24.093750
        Humidity:		65.937500

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 24.093750	pid.out: -10.000000	correctedOutput: -64.820503

	Successfully wrote to file: output.dat

	Simulated 1000 steps out of 1000 total steps
	PID computation total run time: (s): 0.000151
	PID computation total run time (ms): 0.150919
	PID computation total run time (μS): 150.918961
----------------------------------------------------------
	DataPointCount: 11004

        Measurement:
        Timestamp:		1688231777730
        TemperatureCelcius:	24.062500
        Humidity:		65.937500

        Time (s)	System Output		ControllerOutput	CorrectedOutput
        t: 0.000000	measurement: 24.062500	pid.out: -10.000000	correctedOutput: -73.446060

	Successfully wrote to file: output.dat

	Simulated 1000 steps out of 1000 total steps
	PID computation total run time: (s): 0.000204
	PID computation total run time (ms): 0.204086
	PID computation total run time (μS): 204.086304
----------------------------------------------------------
	DataPointCount: 11005
^C	Caught SIGINT, exiting peacefully!

        dummy: 2
        Exiting successfully.
	Memory used in GigaBytes: 0.111759
	Memory used in MegaBytes: 114.440918
Freeing DataPoints: 120000000

        [Performance Metrics]
        Total memory allocated 		 = 0.0 MB
        Current Wall clock run time 	 = 48.6 secs

```

## Dependencies

_How to install required dependencies._

```shell
sudo pacman -S nlohmann-json base-devel cmake
```

## Running

_How to start both programs._

```shell
make
```

## Screenshot

![screenshot.png](screenshot.png "screenshot.png")

<!----------------------------------------------------------------------------->

[badge license]: https://img.shields.io/badge/License-GPL_3-blue.svg?style=for-the-badge
[i2c grove sensor]: https://www.seeedstudio.com/Grove-Temperature-Humidity-Sensor-High-Accuracy-Mini.html
[arduino]: https://docs.arduino.cc/hardware/mkr-1000-wifi
[license]: #
