#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/time.h"

// Ultrasonic HC-SR04
#define TRIG_PIN  2
#define ECHO_PIN  3  // MUST be level-shifted/voltage-divided to 3.3V!

// Servo on PWM-capable pin
#define SERVO_PIN 15
#define SERVO_PWM_FREQ 50.0f // Hz

static void servo_init_pin(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    uint32_t f_sys = 125000000;
    uint32_t top = (uint32_t)(f_sys / (SERVO_PWM_FREQ * 16)) - 1; // clkdiv=16
    pwm_set_clkdiv(slice, 16.0f);
    pwm_set_wrap(slice, top);
    pwm_set_enabled(slice, true);
}

static void servo_write_us(uint pin, float us) {
    uint slice = pwm_gpio_to_slice_num(pin);
    uint32_t top = pwm_hw->slice[slice].top;
    float period_us = 1000000.0f / SERVO_PWM_FREQ; // 20000 us
    uint16_t level = (uint16_t)((us / period_us) * (top + 1));
    pwm_set_gpio_level(pin, level);
}

static void servo_set_angle(float deg) {
    // Typical: 500..2500 us maps to -90..+90 deg
    float us = 1500.0f + (deg / 90.0f) * 1000.0f;
    if (us < 500) us = 500;
    if (us > 2500) us = 2500;
    servo_write_us(SERVO_PIN, us);
}

static bool ultra_measure_mm(uint32_t* out_mm) {
    // Trigger 10 us pulse
    gpio_put(TRIG_PIN, 0);
    sleep_us(2);
    gpio_put(TRIG_PIN, 1);
    sleep_us(10);
    gpio_put(TRIG_PIN, 0);

    // Wait for echo high (timeout)
    absolute_time_t t0 = get_absolute_time();
    while (!gpio_get(ECHO_PIN)) {
        if (absolute_time_diff_us(t0, get_absolute_time()) > 30000) return false;
    }
    uint32_t start = time_us_32();
    // Wait for echo low
    while (gpio_get(ECHO_PIN)) {
        if (absolute_time_diff_us(t0, get_absolute_time()) > 60000) return false;
    }
    uint32_t end = time_us_32();
    uint32_t echo_us = end - start;
    // Convert to mm (speed of sound ~343.2 m/s): distance = (echo_us * 0.3432) / 2 mm
    // mm ≈ echo_us * 0.1716
    *out_mm = (uint32_t)(echo_us * 0.1716f);
    return true;
}

int main() {
    stdio_init_all();
    sleep_ms(300);
    printf("Ultrasonic + Servo demo\n");

    // Ultrasonic pins
    gpio_init(TRIG_PIN); gpio_set_dir(TRIG_PIN, GPIO_OUT); gpio_put(TRIG_PIN, 0);
    gpio_init(ECHO_PIN); gpio_set_dir(ECHO_PIN, GPIO_IN);  // ensure level shifted

    // Servo init
    servo_init_pin(SERVO_PIN);

    const int angles[] = {-30, 0, +30};
    while (true) {
        for (int i = 0; i < 3; ++i) {
            int a = angles[i];
            servo_set_angle((float)a);
            sleep_ms(300); // allow settle
            uint32_t mm = 0;
            bool ok = ultra_measure_mm(&mm);
            printf("Angle %d deg -> %s %lu mm\n", a, ok ? "distance" : "timeout", (unsigned long)mm);
            sleep_ms(300);
        }
    }
}