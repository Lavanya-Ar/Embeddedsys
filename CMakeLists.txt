cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(week6_driver_demo C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pico W for Wi‑Fi demos
set(PICO_BOARD pico_w CACHE STRING "Board type")

pico_sdk_init()

# Build flag for Wi‑Fi demo (OFF until you install a host C++ compiler)
option(BUILD_WIFI "Build WiFi demo (wifi_echo)" OFF)

# 1) WiFi UDP echo (only when BUILD_WIFI=ON)
if(BUILD_WIFI)
  add_executable(wifi_echo wifi_echo.c secrets.h)
  target_link_libraries(wifi_echo pico_stdlib pico_cyw43_arch_lwip_threadsafe_background)
  pico_enable_stdio_usb(wifi_echo 1)
  pico_enable_stdio_uart(wifi_echo 0)
  pico_add_extra_outputs(wifi_echo)
endif()

# 2) Motor + Encoder
add_executable(motor_encoder_demo motor_encoder_demo.c)
target_link_libraries(motor_encoder_demo pico_stdlib hardware_pwm hardware_gpio)
pico_enable_stdio_usb(motor_encoder_demo 1)
pico_enable_stdio_uart(motor_encoder_demo 0)
pico_add_extra_outputs(motor_encoder_demo)

# 3) IR ADC + digital pulse width
add_executable(ir_adc_demo ir_adc_demo.c)
target_link_libraries(ir_adc_demo pico_stdlib hardware_adc hardware_gpio)
pico_enable_stdio_usb(ir_adc_demo 1)
pico_enable_stdio_uart(ir_adc_demo 0)
pico_add_extra_outputs(ir_adc_demo)

# 4) IMU raw (I2C)
add_executable(imu_raw_demo imu_raw_demo.c)
target_link_libraries(imu_raw_demo pico_stdlib hardware_i2c)
pico_enable_stdio_usb(imu_raw_demo 1)
pico_enable_stdio_uart(imu_raw_demo 0)
pico_add_extra_outputs(imu_raw_demo)

# 5) Ultrasonic + Servo
add_executable(ultra_servo_demo ultra_servo_demo.c)
target_link_libraries(ultra_servo_demo pico_stdlib hardware_pwm hardware_gpio)
pico_enable_stdio_usb(ultra_servo_demo 1)
pico_enable_stdio_uart(ultra_servo_demo 0)
pico_add_extra_outputs(ultra_servo_demo)