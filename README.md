# Read Me

Targets (build outputs):
- wifi_echo: UDP echo server (send/receive bytes)
- motor_encoder_demo: PWM motor control both directions + encoder pulse width
- ir_adc_demo: IR sensor raw ADC + BLACK/WHITE classification; digital pulse width capture
- imu_raw_demo: raw accelerometer (e.g., MPU-6050) via I2C
- ultra_servo_demo: servo sweep + HC-SR04 distance

Build
1) Install the Pico SDK and toolchain.
2) Place these files in a new project folder.
3) Copy `secrets.h.example` to `secrets.h` and set SSID/PASS.
4) Configure + build:
   - mkdir build && cd build
   - cmake ..
   - make -j
5) Flash:
   - Hold BOOTSEL, plug Pico W, copy the desired `.uf2` from build/ to RPI-RP2.
   - Replug normally (do not hold BOOTSEL).
   - Open Serial Monitor @115200.

Demo
- wifi_echo: show Pico IP and netcat session sending/receiving a byte.
- motor_encoder_demo: serial logs showing speed steps and encoder pulse width (us), both CW/CCW.
- ir_adc_demo: ADC raw values and BLACK/WHITE labels (hold sensor over line/white).
- imu_raw_demo: streaming accel readings.
- ultra_servo_demo: servo angle and distance (mm) as it sweeps.
