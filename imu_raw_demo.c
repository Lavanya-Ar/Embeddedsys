#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Example for MPU-6050 @ 0x68 (Accel raw)
#define I2C_INST        i2c0
#define I2C_SDA_PIN     4
#define I2C_SCL_PIN     5
#define MPU_ADDR        0x68

static bool mpu_write_reg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    return i2c_write_blocking(I2C_INST, MPU_ADDR, buf, 2, false) == 2;
}

static bool mpu_read_regs(uint8_t start, uint8_t* dst, size_t len) {
    if (i2c_write_blocking(I2C_INST, MPU_ADDR, &start, 1, true) != 1) return false;
    return i2c_read_blocking(I2C_INST, MPU_ADDR, dst, len, false) == (int)len;
}

int main() {
    stdio_init_all();
    sleep_ms(300);
    printf("IMU raw accelerometer demo (MPU-6050)\n");

    i2c_init(I2C_INST, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Wake device (PWR_MGMT_1 = 0)
    if (!mpu_write_reg(0x6B, 0x00)) {
        printf("MPU write fail\n");
    }

    while (true) {
        uint8_t buf[6];
        if (mpu_read_regs(0x3B, buf, 6)) {
            int16_t ax = (int16_t)((buf[0] << 8) | buf[1]);
            int16_t ay = (int16_t)((buf[2] << 8) | buf[3]);
            int16_t az = (int16_t)((buf[4] << 8) | buf[5]);
            printf("ACC raw: ax=%d ay=%d az=%d\n", ax, ay, az);
        } else {
            printf("MPU read fail\n");
        }
        sleep_ms(100);
    }
}