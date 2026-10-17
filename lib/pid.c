#include "pid.h"

void pid_init(pid_ctrl_t *p, float kp, float ki, float kd, float tau,
              float dt, float out_min, float out_max)
{
    p->kp = kp;
    p->ki = ki;
    p->kd = kd;
    p->tau = tau;
    p->dt = dt;
    p->out_min = out_min;
    p->out_max = out_max;
    pid_reset(p, 0.0f);
}

void pid_reset(pid_ctrl_t *p, float meas)
{
    p->integ = 0.0f;
    p->d_term = 0.0f;
    p->prev_err = 0.0f;
    p->prev_meas = meas;
}

static float clampf(float x, float lo, float hi)
{
    return x < lo ? lo : (x > hi ? hi : x);
}

float pid_update(pid_ctrl_t *p, float setpoint, float meas)
{
    float err = setpoint - meas;

    float p_term = p->kp * err;

    /* Tustin integral: area of the trapezoid between the last two errors. */
    float integ_new = p->integ + 0.5f * p->ki * p->dt * (err + p->prev_err);

    /* Filtered derivative of -meas, discretised with Tustin:
     *   D(s) = -kd * s / (tau * s + 1) applied to the measurement. */
    p->d_term = -(2.0f * p->kd * (meas - p->prev_meas)
                  + (2.0f * p->tau - p->dt) * p->d_term)
                / (2.0f * p->tau + p->dt);

    /* Anti-windup (conditional integration): if the output would saturate
     * and the error pushes further into saturation, freeze the integrator. */
    float out = p_term + integ_new + p->d_term;
    if ((out > p->out_max && err > 0.0f) || (out < p->out_min && err < 0.0f))
        out = p_term + p->integ + p->d_term;
    else
        p->integ = integ_new;

    p->prev_err = err;
    p->prev_meas = meas;
    return clampf(out, p->out_min, p->out_max);
}
