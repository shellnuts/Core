/*
 * plant.h - Tiny simulated plants for practising control without hardware.
 *
 * first_order: y' = (K*u - y) / T    (DC motor speed, heater temperature)
 *
 * Swap the simulated plant for a real sensor + actuator later; the
 * controller code does not change. That is the point of the abstraction.
 */
#ifndef PLANT_H
#define PLANT_H

typedef struct {
    float gain;     /* K: steady-state output per unit input */
    float tau;      /* T: time constant [s] */
    float y;        /* output */
} plant_first_order_t;

static inline float plant_first_order_step(plant_first_order_t *pl, float u, float dt)
{
    /* Forward Euler. Fine while dt << tau. */
    pl->y += dt * (pl->gain * u - pl->y) / pl->tau;
    return pl->y;
}

#endif
