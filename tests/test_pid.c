#include "pid.h"
#include "plant.h"
#include "unit.h"

static void test_p_only_output(void)
{
    pid_ctrl_t p;
    pid_init(&p, 2.0f, 0.0f, 0.0f, 0.01f, 0.001f, -100.0f, 100.0f);
    CHECK_NEAR(pid_update(&p, 1.0f, 0.25f), 1.5f, 1e-6);
}

static void test_output_is_clamped(void)
{
    pid_ctrl_t p;
    pid_init(&p, 100.0f, 0.0f, 0.0f, 0.01f, 0.001f, -5.0f, 5.0f);
    CHECK_NEAR(pid_update(&p, 1.0f, 0.0f), 5.0f, 1e-6);
    CHECK_NEAR(pid_update(&p, -1.0f, 0.0f), -5.0f, 1e-6);
}

static void test_integrator_removes_steady_state_error(void)
{
    const float dt = 0.001f;
    plant_first_order_t plant = {.gain = 2.0f, .tau = 0.5f, .y = 0.0f};
    pid_ctrl_t p;
    pid_init(&p, 2.0f, 4.0f, 0.05f, 0.01f, dt, -10.0f, 10.0f);
    for (int k = 0; k < 5000; k++)
        plant_first_order_step(&plant, pid_update(&p, 1.0f, plant.y), dt);
    CHECK_NEAR(plant.y, 1.0, 1e-3);
}

static void test_no_derivative_kick_on_setpoint_step(void)
{
    pid_ctrl_t p;
    pid_init(&p, 0.0f, 0.0f, 1.0f, 0.01f, 0.001f, -1e6f, 1e6f);
    pid_reset(&p, 0.0f);
    /* Setpoint jumps, measurement constant: derivative-on-measurement = 0. */
    CHECK_NEAR(pid_update(&p, 100.0f, 0.0f), 0.0f, 1e-6);
}

/* Saturated actuator for a long time, then a setpoint drop. Without
 * anti-windup the integrator would hold the output at max for ages. */
static void test_anti_windup_recovers_quickly(void)
{
    const float dt = 0.001f;
    plant_first_order_t plant = {.gain = 1.0f, .tau = 0.5f, .y = 0.0f};
    pid_ctrl_t p;
    pid_init(&p, 1.0f, 5.0f, 0.0f, 0.01f, dt, -1.0f, 1.0f);

    /* Unreachable setpoint: plant can only reach gain * out_max = 1.0. */
    for (int k = 0; k < 10000; k++)
        plant_first_order_step(&plant, pid_update(&p, 5.0f, plant.y), dt);
    CHECK(p.integ <= 1.0f + 1e-3f);

    /* Setpoint now reachable: output must leave saturation promptly. */
    int k;
    for (k = 0; k < 2000; k++) {
        float u = pid_update(&p, 0.5f, plant.y);
        plant_first_order_step(&plant, u, dt);
        if (u < 1.0f)
            break;
    }
    CHECK(k < 50);
}

int main(void)
{
    RUN(test_p_only_output);
    RUN(test_output_is_clamped);
    RUN(test_integrator_removes_steady_state_error);
    RUN(test_no_derivative_kick_on_setpoint_step);
    RUN(test_anti_windup_recovers_quickly);
    return UNIT_REPORT();
}
