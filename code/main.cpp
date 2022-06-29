#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <algorithm>
#include <random>
#include <climits>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

// Cpp spec does shenanigans with the usage of <static>
#define internal static
#define local_persist static
#define global_variable static

// Program constants
long int MemoryAllocatedCPU = 0L;
const float PI = acos(-1);

const char *portname = "/dev/ttyACM0";

int set_interface_attribs(int fd, int speed, int parity)
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

void set_blocking(int fd, int should_block)
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

int main(int argc, char *argv[])
{
    // Time measurement
    struct timeval _ttime;
    struct timezone _tzone;
    gettimeofday(&_ttime, &_tzone);
    double time_start = (double)_ttime.tv_sec + (double)_ttime.tv_usec / 1000000.;

    // Unused input arguments
    if (argc > 0)
    {
        for (int i = 0; i < argc; ++i)
        {
            printf("%s\n", argv[i]);
        }
    }

    // Read data from the Arduinos Serial port in the Linux host
    int fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        printf("error %d opening %s: %s", errno, portname, strerror(errno));
        return (1);
    }

    int set_interface_attribs_result = set_interface_attribs(fd, B9600, 0); // set speed to 9600 bps, 8n1 (no parity)
    printf("set_interface_attribs_result: %d", set_interface_attribs_result);
    set_blocking(fd, 0); // set no blocking

    // write(fd, "hello!\n", 7); // send 7 character greeting

    bool running = true;
    unsigned long long int masterCount = 0;

    while (running)
    {
        usleep((100) * 100);

        char buf[100];
        int n = read(fd, buf, sizeof buf); // read up to 100 characters if ready to read

        for (int i = 0; i < n; ++i)
        {
            char rawBuffer = buf[i];

            printf("%c", rawBuffer);

            /* for (int j = 0; j < sizeof(rawBuffer); ++j)
            {
                char line = rawBuffer[j];
                printf("%c", line);
            } */
        }

        time_t rawtime;
        struct tm *timeinfo;

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        if (masterCount % 10 == 0)
        {
            printf("\nCurrent local time and date: %s\n", asctime(timeinfo));
        }

        ++masterCount;
    }

    // Performance metrics
    printf("   Total memory allocated = %.1lf MB\n", MemoryAllocatedCPU / 1000000.0);
    gettimeofday(&_ttime, &_tzone);
    double time_end = (double)_ttime.tv_sec + (double)_ttime.tv_usec / 1000000.;

    printf("   Wall clock run time    = %.1lf secs\n", time_end - time_start);

    return (0);
}