#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/time.h"

// ---------------- Motor pins ----------------
#define M1A 8   // Motor 1 forward
#define M1B 9   // Motor 1 backward
#define M2A 10  // Motor 2 forward
#define M2B 11  // Motor 2 backward

// ---------------- Encoder pins ----------------
#define ENC1_DIG 6     // Motor 1 encoder digital
#define ENC1_ADC 26    // Motor 1 encoder analog (ADC0)
#define ENC2_DIG 4     // Motor 2 encoder digital
#define ENC2_DIG_B 5   // (optional second channel if available)

// ---------------- PWM config ----------------
#define F_PWM_HZ 20000.0f
#define SYS_CLK_HZ 125000000u

// ---------------- Encoder state ----------------
static volatile uint32_t enc1_last_rise=0, enc1_period=0, enc1_high=0;
static volatile uint32_t enc2_last_rise=0, enc2_period=0, enc2_high=0;

static inline uint32_t now_us(void){ return time_us_32(); }

void enc1_isr(uint gpio, uint32_t events){
    uint32_t t = now_us();
    if (events & GPIO_IRQ_EDGE_RISE) {
        if (enc1_last_rise) enc1_period = t - enc1_last_rise;
        enc1_last_rise = t;
    }
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (enc1_last_rise) enc1_high = t - enc1_last_rise;
    }
}

void enc2_isr(uint gpio, uint32_t events){
    uint32_t t = now_us();
    if (events & GPIO_IRQ_EDGE_RISE) {
        if (enc2_last_rise) enc2_period = t - enc2_last_rise;
        enc2_last_rise = t;
    }
    if (events & GPIO_IRQ_EDGE_FALL) {
        if (enc2_last_rise) enc2_high = t - enc2_last_rise;
    }
}

// ---------------- PWM helpers ----------------
static void setup_pwm(uint pin, float freq_hz){
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    uint32_t wrap = 9999;
    float div = (float)SYS_CLK_HZ / (freq_hz * (wrap + 1));
    pwm_set_clkdiv(slice, div);
    pwm_set_wrap(slice, wrap);
    pwm_set_gpio_level(pin, 0);
    pwm_set_enabled(slice, true);
}

static inline void set_pwm_pct(uint pin, float pct){
    if (pct < 0) pct = 0; if (pct > 100) pct = 100;
    uint slice = pwm_gpio_to_slice_num(pin);
    uint32_t top = pwm_hw->slice[slice].top;
    pwm_set_gpio_level(pin, (uint16_t)((pct/100.0f) * (top + 1)));
}

// ---------------- Motor control ----------------
static void motor1_forward(float pct){ set_pwm_pct(M1A, pct); set_pwm_pct(M1B, 0); }
static void motor1_backward(float pct){ set_pwm_pct(M1A, 0); set_pwm_pct(M1B, pct); }
static void motor2_forward(float pct){ set_pwm_pct(M2A, pct); set_pwm_pct(M2B, 0); }
static void motor2_backward(float pct){ set_pwm_pct(M2A, 0); set_pwm_pct(M2B, pct); }
static void motors_stop(void){ set_pwm_pct(M1A,0); set_pwm_pct(M1B,0); set_pwm_pct(M2A,0); set_pwm_pct(M2B,0); }

// ---------------- Main ----------------
int main(){
    stdio_init_all();
    sleep_ms(300);
    printf("\n[Motor + Encoder Demo]\n");

    // Init PWM
    setup_pwm(M1A); setup_pwm(M1B);
    setup_pwm(M2A); setup_pwm(M2B);

    // Init ADC for IR analog
    adc_init();
    adc_gpio_init(ENC1_ADC);   // GP26

    // Init encoders digital
    gpio_init(ENC1_DIG); gpio_set_dir(ENC1_DIG, GPIO_IN); gpio_pull_up(ENC1_DIG);
    gpio_set_irq_enabled_with_callback(ENC1_DIG, GPIO_IRQ_EDGE_RISE|GPIO_IRQ_EDGE_FALL, true, &enc1_isr);

    gpio_init(ENC2_DIG); gpio_set_dir(ENC2_DIG, GPIO_IN); gpio_pull_up(ENC2_DIG);
    gpio_set_irq_enabled_with_callback(ENC2_DIG, GPIO_IRQ_EDGE_RISE|GPIO_IRQ_EDGE_FALL, true, &enc2_isr);

    while (true) {
        // Forward sweep
        printf("Forward sweep\n");
        for (int s=20; s<=100; s+=20) {
            motor1_forward((float)s);
            motor2_forward((float)s);
            adc_select_input(0);
            uint16_t ir_val = adc_read();
            printf("Speed %d%% | ENC1 high=%lu us period=%lu us | ENC2 high=%lu us period=%lu us | IR_ADC=%u\n",
                   s, enc1_high, enc1_period, enc2_high, enc2_period, ir_val);
            sleep_ms(1000);
        }

        motors_stop(); sleep_ms(1500);

        // Backward sweep
        printf("Backward sweep\n");
        for (int s=20; s<=100; s+=20) {
            motor1_backward((float)s);
            motor2_backward((float)s);
            adc_select_input(0);
            uint16_t ir_val = adc_read();
            printf("Speed %d%% | ENC1 high=%lu us period=%lu us | ENC2 high=%lu us period=%lu us | IR_ADC=%u\n",
                   s, enc1_high, enc1_period, enc2_high, enc2_period, ir_val);
            sleep_ms(1000);
        }

        motors_stop(); sleep_ms(2000);
    }
}