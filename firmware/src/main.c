/*
 * main.c - Starter application for the ALIENTEK Explorer STM32F407.
 *
 * What it does:
 *   - 168 MHz clock, 1 ms SysTick, USART1 at 115200 baud (USB_232 port)
 *   - LED0 blinks at 1 Hz (proof of life), LED1 mirrors KEY0, WK_UP beeps
 *   - a tiny command shell on the serial port: 'h' for help
 *   - 's' runs a PID step response against a simulated plant and streams
 *     CSV that tools/plot_telemetry.py can plot: your first control loop,
 *     no wiring needed.
 */
#include <stdio.h>

#include "board.h"
#include "clock.h"
#include "pid.h"
#include "plant.h"
#include "systick.h"
#include "uart.h"

static void print_help(void)
{
    printf("\nCommands:\n"
           "  h  this help\n"
           "  s  PID step-response simulation (CSV: t,setpoint,y,u)\n"
           "  i  system info\n");
}

static void print_info(void)
{
    printf("SYSCLK %lu Hz, PCLK1 %lu Hz, PCLK2 %lu Hz, uptime %lu ms\n",
           (unsigned long)g_sysclk_hz, (unsigned long)g_pclk1_hz,
           (unsigned long)g_pclk2_hz, (unsigned long)millis());
}

/*
 * Closed-loop simulation at 1 kHz. In real firmware this body would run in
 * a timer ISR or an RTOS task; here it runs as fast as the CPU allows and
 * only uses 'dt' as simulated time.
 */
static void run_step_sim(void)
{
    const float dt = 0.001f;
    plant_first_order_t plant = {.gain = 2.0f, .tau = 0.5f, .y = 0.0f};
    pid_ctrl_t pid;
    pid_init(&pid, 2.0f, 4.0f, 0.05f, 0.01f, dt, -10.0f, 10.0f);
    pid_reset(&pid, plant.y);

    printf("t,setpoint,y,u\n");
    for (int k = 0; k <= 3000; k++) {
        float t = (float)k * dt;
        float sp = (t >= 0.1f) ? 1.0f : 0.0f;
        float u = pid_update(&pid, sp, plant.y);
        plant_first_order_step(&plant, u, dt);
        if (k % 10 == 0) /* decimate: 100 Hz is plenty for plotting */
            printf("%.3f,%.3f,%.4f,%.4f\n", (double)t, (double)sp, (double)plant.y, (double)u);
    }
    printf("# done\n");
}

int main(void)
{
    bool hse_ok = clock_init();
    systick_init(g_sysclk_hz);
    board_init();
    uart1_init(115200);

    printf("\n=== STM32F407 Explorer starter ===\n");
    if (!hse_ok)
        printf("warning: HSE crystal failed, running from HSI\n");
    print_info();
    print_help();

    uint32_t last_blink = millis();
    for (;;) {
        if (millis() - last_blink >= 500u) {
            last_blink += 500u;
            board_led_toggle(LED0);
        }

        board_led_set(LED1, board_key_pressed(KEY0));
        board_beep(board_key_pressed(KEY_WKUP));

        char c;
        if (uart1_getc(&c)) {
            switch (c) {
            case 'h': print_help(); break;
            case 'i': print_info(); break;
            case 's': run_step_sim(); break;
            case '\r':
            case '\n': break;
            default: printf("unknown '%c', press h\n", c); break;
            }
        }
    }
}
