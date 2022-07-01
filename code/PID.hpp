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

#pragma once

struct PID
{
    double Kp;       // Proportional
    double Ki;       // Integral
    double Kd;       // Derivative
    double setPoint; // What we want the input to be

    double processValue;    // Input to the PID-controller
    double correctedOutput; // Output from the PID-controller

    void initialize()
    {
        Kp = 0.0;
        Ki = 0.0;
        Kd = 0.0;

        setPoint = 0.0;

        processValue = 0.0;
        correctedOutput = 0.0;
    }

    void update()
    {
        printf("        Update\n");
    }
};