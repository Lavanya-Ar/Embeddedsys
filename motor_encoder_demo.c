#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/time.h"

// Motor pins (adjust to your driver wiring)
#define PWM_LEFT   6
#define PWM_RIGHT  7
#define L_IN1      8
#define L_IN2      9
#define R_IN3      10
#define R_IN4      11

// Encoder input (single channel demo)
#define ENC_PIN    14

static volatile uint32_t last_rise_us = 0;
static volatile uint32_t last_fall_us = 0;
static volatile uint32_t high_us = 0;   // last measured high pulse width
static volatile uint32_t period_us = 0; // last measured period (rise to rise)

void enc_isr(uint gpio, uint32_t events) {
    uint32_t now = time_us_32();
    if (events & GPIO_IRQ_EDGE_RISE) {
        if (last_rise_us) period_us = now - last_rise_us;
        last_rise_us = now;
    }
    if (events & GPIO_IRQ_EDGE_FALL) {
        last_fall_us = now;
        if (last_rise_us) high_us = last_fall_us - last_rise_us;
    }
}

static void pwm_init_pin(uint pin, float freq_hz) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    // target ~20 kHz PWM
    if (freq_hz <= 0) freq_hz = 20000.0f;
    uint32_t f_sys = 125000000;
    uint32_t wrap = 9999; // simple config
    float div = (float)f_sys / (freq_hz * (wrap + 1));
    pwm_set_clkdiv(slice, div);
    pwm_set_wrap(slice, wrap);
    pwm_set_gpio_level(pin, 0);
    pwm_set_enabled(slice, true);
}

static void motor_set(float left_pct, float right_pct) {
    // clamp
    if (left_pct > 100) left_pct = 100; if (left_pct < -100) left_pct = -100;
    if (right_pct > 100) right_pct = 100; if (right_pct < -100) right_pct = -100;

    // Direction
    bool l_fwd = left_pct >= 0;
    bool r_fwd = right_pct >= 0;

    gpio_put(L_IN1, l_fwd); gpio_put(L_IN2, !l_fwd);
    gpio_put(R_IN3, r_fwd); gpio_put(R_IN4, !r_fwd);

    // Duty
    uint sliceL = pwm_gpio_to_slice_num(PWM_LEFT);
    uint sliceR = pwm_gpio_to_slice_num(PWM_RIGHT);
    uint32_t wrap = pwm_hw->slice[sliceL].top; // same wrap used
    uint16_t l = (uint16_t)(wrap * (float) ( (l_fwd? left_pct : -left_pct) / 100.0f ));
    uint16_t r = (uint16_t)(wrap * (float) ( (r_fwd? right_pct : -right_pct) / 100.0f ));
    pwm_set_gpio_level(PWM_LEFT, l);
    pwm_set_gpio_level(PWM_RIGHT, r);
}

int main() {
    stdio_init_all();
    sleep_ms(300);
    printf("Motor + Encoder demo\n");

    // Direction pins
    gpio_init(L_IN1); gpio_set_dir(L_IN1, GPIO_OUT);
    gpio_init(L_IN2); gpio_set_dir(L_IN2, GPIO_OUT);
    gpio_init(R_IN3); gpio_set_dir(R_IN3, GPIO_OUT);
    gpio_init(R_IN4); gpio_set_dir(R_IN4, GPIO_OUT);

    // PWM pins
    pwm_init_pin(PWM_LEFT, 20000.0f);
    pwm_init_pin(PWM_RIGHT, 20000.0f);

    // Encoder input with IRQ on both edges
    gpio_init(ENC_PIN);
    gpio_set_dir(ENC_PIN, GPIO_IN);
    gpio_pull_up(ENC_PIN);
    gpio_set_irq_enabled_with_callback(ENC_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &enc_isr);

    // Ramp sequence: CW → stop → CCW
    while (true) {
        for (int s = 0; s <= 100; s += 10) {
            motor_set(s, s);
            printf("Speed %+3d%% | pulse high=%lu us period=%lu us\n", s, (unsigned long)high_us, (unsigned long)period_us);
            sleep_ms(500);
        }
        motor_set(0,0); sleep_ms(500);

        for (int s = 0; s >= -100; s -= 10) {
            motor_set(s, s);
            printf("Speed %+3d%% | pulse high=%lu us period=%lu us\n", s, (unsigned long)high_us, (unsigned long)period_us);
            sleep_ms(500);
        }
        motor_set(0,0); sleep_ms(1000);
    }
}