#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/time.h"

// IR analog input (reflectance): GP26 = ADC0
#define IR_ADC_INPUT   0  // 0=>GP26, 1=>GP27, 2=>GP28

// Optional digital pulse-width input (use same pin as encoder, or another digital source)
#define PW_IN_PIN      14

static volatile uint32_t rise_us = 0, fall_us = 0, high_us = 0;

void pw_isr(uint gpio, uint32_t events) {
    uint32_t now = time_us_32();
    if (events & GPIO_IRQ_EDGE_RISE) rise_us = now;
    if (events & GPIO_IRQ_EDGE_FALL) { fall_us = now; high_us = fall_us - rise_us; }
}

int main() {
    stdio_init_all();
    sleep_ms(300);
    printf("IR ADC + digital pulse width demo\n");

    // ADC init
    adc_init();
    adc_select_input(IR_ADC_INPUT);

    // Digital pulse-width ISR (optional)
    gpio_init(PW_IN_PIN);
    gpio_set_dir(PW_IN_PIN, GPIO_IN);
    gpio_pull_up(PW_IN_PIN);
    gpio_set_irq_enabled_with_callback(PW_IN_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &pw_isr);

    // Simple dynamic threshold (update over first seconds if needed)
    uint16_t thresh = 2000;

    while (true) {
        uint16_t raw = adc_read();
        const char* cls = (raw > thresh) ? "WHITE" : "BLACK";
        printf("ADC raw=%u -> %s | digital high_us=%lu\n", raw, cls, (unsigned long)high_us);
        sleep_ms(100);
    }
}