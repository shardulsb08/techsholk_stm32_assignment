#ifndef __MPU6050_H
#define __MPU6050_H

#include "main.h" // Includes stm32l0xx_hal.h and defines hi2c1

// --- MPU6050 Register Addresses ---
#define MPU6050_ADDR        0xD0 // MPU6050 I2C 8-bit address (AD0=LOW: 0x68 << 1)
#define MPU_WHO_AM_I_REG    0x75
#define MPU_PWR_MGMT_1_REG  0x6B
#define MPU_ACCEL_XOUT_H_REG 0x3B // Start of 14-byte data burst

// --- Configuration Values ---
#define MPU_CLOCK_SOURCE_PLL_X 0x01 // Use PLL with X-axis Gyro reference
#define MPU_GYRO_CONFIG_250DPS 0x00 // Full scale range: +/- 250 deg/s
#define MPU_ACCEL_CONFIG_2G    0x00 // Full scale range: +/- 2g
#define MPU_IIR_ALPHA          0.2f // First-order IIR filter constant

// --- Scale Factors (Based on 2g and 250 deg/s configuration) ---
// Sensitivity is 16384 LSB/g for 2g range
#define ACCEL_SCALE_FACTOR     16384.0f
// Sensitivity is 131 LSB/(deg/s) for 250 deg/s range
#define GYRO_SCALE_FACTOR      131.0f

// Structure to hold raw and filtered data
typedef struct {
    int16_t accel_raw[3]; // X, Y, Z
    int16_t gyro_raw[3];  // X, Y, Z

    float accel_g[3];     // Filtered Accel in 'g'
    float gyro_dps[3];    // Filtered Gyro in 'degrees/second'

    uint8_t sensor_status; // 0=OK, 1=Unavailable
} MPU6050_Data_t;

extern MPU6050_Data_t g_mpu_data;
extern I2C_HandleTypeDef hi2c1; // Defined in main.c by CubeMX

// Function Prototypes
uint8_t mpu6050_init(void);
uint8_t mpu6050_read_and_filter(void);
void mpu6050_print_filtered(void);

#endif // __MPU6050_H
