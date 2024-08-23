#include "PID.hpp"

void PIDControllerInitialize(PIDController *pid)
{

    // Clear controller variables
    pid->integrator = 0.0f;
    pid->prevError = 0.0f;

    pid->differentiator = 0.0f;
    pid->prevMeasurement = 0.0f;

    // Tune with Siegler-Nichols method
    // pid->Kp = 0.6f * Ku;
    // pid->Ki = 1.2f * Ku / Tu;
    // pid->Kd = 3.0f * Ku * Tu / 40.0f;

    // Set gains
    pid->Kp = 2.0f;
    pid->Ki = 0.5f;
    pid->Kd = 0.25f;

    // Set time constant
    pid->tau = 0.02f;

    // Set output limits
    pid->limMin = -10.0f;
    pid->limMax = 10.0f;

    // Set integrator limits
    pid->limMinInt = -5.0f;
    pid->limMaxInt = 5.0f;

    // Set sample time
    pid->T = 0.001f;
}

float PIDControllerUpdate(PIDController *pid, float setpoint, float measurement)
{
    // Error signal
    float error = setpoint - measurement;

    // Proportional
    float proportional = pid->Kp * error;

    // Integral
    pid->integrator = pid->integrator + 0.5f * pid->Ki * pid->T * (error + pid->prevError);

    //  Anti-wind-up via non branching integrator clamping
    pid->integrator > pid->limMaxInt ? pid->integrator = pid->limMaxInt : pid->integrator = pid->limMinInt;

    // Derivative (band-limited differentiator)
    pid->differentiator = -(2.0f * pid->Kd * (measurement - pid->prevMeasurement) // Note: derivative on measurement, therefore minus sign in front of equation!
                            + (2.0f * pid->tau - pid->T) * pid->differentiator) /
                          (2.0f * pid->tau + pid->T);

    // Compute output and apply limits
    pid->out = proportional + pid->integrator + pid->differentiator;

    if (pid->out > pid->limMax)
    {
        pid->out = pid->limMax;
    }
    else if (pid->out < pid->limMin)
    {
        pid->out = pid->limMin;
    }

    // Store error and measurement for later use
    pid->prevError = error;
    pid->prevMeasurement = measurement;

    // Return controller output
    return (pid->out);
}