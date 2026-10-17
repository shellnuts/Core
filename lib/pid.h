/*
 * pid.h - Discrete PID controller for embedded use.
 *
 * Features that separate a usable PID from the textbook formula:
 *   - trapezoidal (Tustin) integration
 *   - derivative on measurement (no "derivative kick" on setpoint steps)
 *   - first-order low-pass on the derivative (time constant 'tau')
 *   - output clamping with conditional-integration anti-windup
 *
 * Call pid_update() at a FIXED rate of 1/dt from a timer interrupt or a
 * periodic RTOS task. Timing jitter appears directly as control error.
 */
#ifndef PID_H
#define PID_H

typedef struct {
    /* configuration */
    float kp, ki, kd;
    float tau;              /* derivative filter time constant [s] */
    float dt;               /* sample period [s] */
    float out_min, out_max; /* actuator limits */

    /* state */
    float integ;
    float d_term;
    float prev_err;
    float prev_meas;
} pid_ctrl_t;

void pid_init(pid_ctrl_t *p, float kp, float ki, float kd, float tau,
              float dt, float out_min, float out_max);

/* Clear the state; pass the current measurement to avoid a derivative spike. */
void pid_reset(pid_ctrl_t *p, float meas);

/* Returns the actuator command, already clamped to [out_min, out_max]. */
float pid_update(pid_ctrl_t *p, float setpoint, float meas);

#endif
