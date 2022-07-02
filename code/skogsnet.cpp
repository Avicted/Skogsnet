#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <algorithm>
#include <random>
#include <climits>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include "json.hpp"
using json = nlohmann::json;
#include "PID.hpp"

// Cpp spec does shenanigans with the usage of <static>, let's be explicit in what
// we mean with 'static'
#define internal static
#define local_persist static
#define global_variable static

// Program constants ------------------------------------------------
global_variable long int MemoryAllocatedCPU = 0L;
global_variable const float PI = acos(-1);

global_variable struct timeval _ttime;
global_variable struct timezone _tzone;
global_variable double time_start;
global_variable double program_time_start;

global_variable bool running;

std::string portname = "/dev/ttyACM";

// Controller parameters
#define PID_KP 2.0f
#define PID_KI 0.5f
#define PID_KD 0.25f

#define PID_TAU 0.02f

#define PID_LIM_MIN -10.0f
#define PID_LIM_MAX 10.0f

#define PID_LIM_MIN_INT -5.0f
#define PID_LIM_MAX_INT 5.0f

#define SAMPLE_TIME_S 0.0000001

// Maximum run-time of simulation
#define SIMULATION_TIME_MAX 1.0
// ------------------------------------------------------------------

internal float TestSystem_Update(float inp)
{
    static float output = 0.0f;
    static const float alpha = 0.02f;

    output = (SAMPLE_TIME_S * inp + output) / (1.0f + alpha * SAMPLE_TIME_S);

    return output;
}

struct Measurement
{
    std::string Timestamp;
    float TemperatureCelcius;
    float Humidity;

    void print()
    {
        printf("\n        Measurement:\n");
        printf("        Timestamp:\t\t%s\n", Timestamp.c_str());
        printf("        TemperatureCelcius:\t%f\n", TemperatureCelcius);
        printf("        Humidity:\t\t%f\n", Humidity);
    }
};

internal int
set_interface_attribs(int fd, int speed, int parity)
{
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0)
    {
        printf("error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed(&tty, speed);
    cfsetispeed(&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK; // disable break processing
    tty.c_lflag = 0;        // no signaling chars, no echo,
                            // no canonical processing
    tty.c_oflag = 0;        // no remapping, no delays
    tty.c_cc[VMIN] = 0;     // read doesn't block
    tty.c_cc[VTIME] = 5;    // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);   // ignore modem controls,
                                       // enable reading
    tty.c_cflag &= ~(PARENB | PARODD); // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        printf("error %d from tcsetattr", errno);
        return -1;
    }
    return 0;
}

internal void
set_blocking(int fd, int should_block)
{
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(fd, &tty) != 0)
    {
        printf("error %d from tggetattr", errno);
        return;
    }

    tty.c_cc[VMIN] = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5; // 0.5 seconds read timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
        printf("error %d setting term attributes", errno);
}

internal void
print_performance_metrics()
{
    printf("\n        [Performance Metrics]\n");
    printf("        Total memory allocated \t\t = %.1lf MB\n", MemoryAllocatedCPU / 1000000.0);
    gettimeofday(&_ttime, &_tzone);
    double time_end = (double)_ttime.tv_sec + (double)_ttime.tv_usec / 1000000.;

    printf("        Current Wall clock run time \t = %.1lf secs\n", time_end - program_time_start);
}

internal void
intHandler(int dummy)
{
    running = false;
    printf("\n        dummy: %d\n", dummy);
    printf("        Exiting successfully.\n");
}

internal Measurement
deserializeJSON(char *buffer)
{
    Measurement newMeasurement;

    // JSON deserializer
    try
    {
        // fill a stream with JSON text
        std::stringstream ss;
        ss << buffer;

        // parse and serialize JSON
        json j_complete = json::parse(ss);
        std::cout << std::setw(4) << j_complete << "\n\n";

        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%d.%m.%Y %T");
        std::string dateTimeNow = oss.str();

        newMeasurement.Timestamp = dateTimeNow;
        newMeasurement.TemperatureCelcius = 0.0f;
        newMeasurement.Humidity = 0.0f;

        // special iterator member functions for objects
        for (json::iterator it = j_complete.begin(); it != j_complete.end(); ++it)
        {
            // std::cout << it.key() << " : " << it.value() << "\n";

            if (it.key() == "temperature_celcius")
            {
                newMeasurement.TemperatureCelcius = it.value();
            }
            if (it.key() == "humidity")
            {
                newMeasurement.Humidity = it.value();
            }
        }

        // @Note(Victor): the newMeasurement members are now populated
        // Use the Measurement to act on actuators based on some logic for example.
        newMeasurement.print();
    }
    catch (json::exception &e)
    {
        std::cerr << "\n      ERROR: Could not parse the JSON from the serial message." << '\n';
        std::cerr << e.what() << std::endl;
    }

    return (newMeasurement);
}

internal void
initializeSerialCommunication(int fd, unsigned int connectionAttempts, unsigned int maxConnectionAttempts)
{
    // Read data from the Arduinos Serial port in the Linux host
    // Let us try five ports lol
    std::string portnameString = portname + std::to_string(connectionAttempts);
    fd = open(portnameString.c_str(), O_RDWR | O_NOCTTY | O_SYNC); // file descriptor

    while (connectionAttempts < maxConnectionAttempts)
    {
        MemoryAllocatedCPU += 1L * sizeof(portnameString);

        portnameString = portname + std::to_string(connectionAttempts);

        std::cout << "\n      Trying port: " << portnameString << std::endl;

        fd = open(portnameString.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

        if (fd < 0)
        {
            printf("error %d opening %s: %s", errno, portnameString.c_str(), strerror(errno));
            ++connectionAttempts;

            int secondsToSleep = 1;
            sleep(secondsToSleep);

            continue;
        }

        connectionAttempts = 1000;

        set_interface_attribs(fd, B115200, 0); // set speed to 115200 bps, 8n1 (no parity)
        set_blocking(fd, 0);                   // set no blocking
    }

    printf("\n        Skogsnet is running now, connected to port: %s\n\n", portnameString.c_str());
}

int main(int argc, char *argv[])
{
    printf("        Settings up time measurement and serial communications...\n");

    // Time measurement
    gettimeofday(&_ttime, &_tzone);
    program_time_start = (double)_ttime.tv_sec + (double)_ttime.tv_usec / 1000000.;

    // Signals
    signal(SIGINT, intHandler);

    // Unused input arguments
    if (argc > 0)
    {
        for (int i = 0; i < argc; ++i)
        {
            printf("%s\n", argv[i]);
        }
    }

    // Read data from the Arduinos Serial port in the Linux host
    // Let us try five ports lol
    unsigned int connectionAttempts = 0;
    unsigned int maxConnectionAttempts = 5;
    std::string portnameString = portname + std::to_string(connectionAttempts);
    int fd = open(portnameString.c_str(), O_RDWR | O_NOCTTY | O_SYNC); // file descriptor

    initializeSerialCommunication(fd, connectionAttempts, maxConnectionAttempts);

    // Initialise PID controller
    PIDController pid = {
        PID_KP,
        PID_KI,
        PID_KD,
        PID_TAU,
        PID_LIM_MIN,
        PID_LIM_MAX,
        PID_LIM_MIN_INT,
        PID_LIM_MAX_INT,
        SAMPLE_TIME_S,
    };

    PIDControllerInitialize(&pid);

    // Simulate response using test system
    float setpoint = 20.0f;

    // Main loop
    running = true;
    while (running)
    {
        // Read data ---------------------------------------------------
        unsigned int bufferSize = 256;
        char buffer[bufferSize] = {0};
        unsigned int pos = 0;

        MemoryAllocatedCPU += 1L * bufferSize * sizeof(char);

        usleep(10000);

        // Read 1 byte at a time from the serial port
        while (read(fd, buffer + pos, 1))
        {
            if (pos >= bufferSize)
            {
                continue;
            }
            if (buffer[pos] == '\n')
            {
                break;
            }

            ++pos;
        }

        // Check for an empty buffer
        if (strlen(buffer) == 0)
        {
            continue;
        }

        // Deserialize data -----------------------------------------------
        Measurement newMeasurement = deserializeJSON(buffer);

        // PID control the based on the data ------------------------------
        gettimeofday(&_ttime, &_tzone);
        double pid_time_start = (double)_ttime.tv_sec + (double)_ttime.tv_usec / 1000000.;

        printf("\n        Time (s)\tSystem Output\t\tControllerOutput\tCorrectedOutput\r\n");
        unsigned long int steps = 0;

        for (float t = 0.0f; t <= SIMULATION_TIME_MAX; t += SAMPLE_TIME_S)
        {
            // Get measurement from system
            float correctedOutput = TestSystem_Update(pid.out);

            // Compute new control signal
            PIDControllerUpdate(&pid, setpoint, newMeasurement.TemperatureCelcius);

            if (t == 0)
            {
                printf("        t: %f\tmeasurement: %f\tpid.out: %f\tcorrectedOutput: %f\r\n", t, newMeasurement.TemperatureCelcius, pid.out, correctedOutput);
            }

            gettimeofday(&_ttime, &_tzone);
            double time_end = (double)_ttime.tv_sec + (double)_ttime.tv_usec / 1000000.;
            if (pid_time_start - time_end >= 1.0)
            {
                printf("        Warning: PID computation took too long time_end: %lf!\n", pid_time_start - time_end);
                printf("        t: %f\n", t);
                break;
            }

            ++steps;
        }

        gettimeofday(&_ttime, &_tzone);
        double time_end = (double)_ttime.tv_sec + (double)_ttime.tv_usec / 1000000.;

        unsigned long int totalSteps = (double)SIMULATION_TIME_MAX / (double)SAMPLE_TIME_S;
        printf("\n        Simulated %ld steps out of %ld total steps\n", steps, totalSteps);
        printf("        PID computation run time = %.1lf secs out of our time budget: %f\n", (time_end - pid_time_start), SIMULATION_TIME_MAX);
    }

    print_performance_metrics();

    return (0);
}
