#include "mpu6050.h"
#include <stdio.h>
#include <math.h>

MPU6050_Data_t g_mpu_data = {0}; // Global instance of sensor data

/**
 * @brief Applies a first-order IIR Low-Pass filter.
 * y[n] = y[n-1] + alpha * (x[n] - y[n-1])
 * @param x_new: New raw value.
 * @param y_prev: Pointer to the previous filtered value (will be updated to y[n]).
 * @param alpha: Filter constant.
 */
static void apply_iir_filter(float x_new, float *y_prev, float alpha) {
    *y_prev = *y_prev + alpha * (x_new - *y_prev);
}

/**
 * @brief Initializes the MPU6050 sensor.
 * @retval 0 on success, 1 on error.
 */
uint8_t mpu6050_init(void) {
    uint8_t data[2];

    // 1. Check if the device is ready
    if (HAL_I2C_IsDeviceReady(&hi2c1, MPU6050_ADDR, 3, 100) != HAL_OK) {
        g_mpu_data.sensor_status = 1;
        return 1; // Device not found/ready
    }

    // 2. Wake up the sensor: Write 0x00 to PWR_MGMT_1 (0x6B)
    data[0] = MPU_CLOCK_SOURCE_PLL_X; // Set clock source, clear SLEEP bit
    if (HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, MPU_PWR_MGMT_1_REG, 1, data, 1, 100) != HAL_OK) {
        g_mpu_data.sensor_status = 1;
        return 1;
    }

    // 3. Configure Gyroscope: Write 0x00 to GYRO_CONFIG (0x1B) for +/- 250 deg/s
    data[0] = MPU_GYRO_CONFIG_250DPS;
    if (HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x1B, 1, data, 1, 100) != HAL_OK) {
        g_mpu_data.sensor_status = 1;
        return 1;
    }

    // 4. Configure Accelerometer: Write 0x00 to ACCEL_CONFIG (0x1C) for +/- 2g
    data[0] = MPU_ACCEL_CONFIG_2G;
    if (HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, 0x1C, 1, data, 1, 100) != HAL_OK) {
        g_mpu_data.sensor_status = 1;
        return 1;
    }

    g_mpu_data.sensor_status = 0;
    return 0;
}

/**
 * @brief Reads all 14 bytes, converts, and applies the IIR filter.
 * @retval 0 on success, 1 on I2C error.
 */
uint8_t mpu6050_read_and_filter(void) {
    uint8_t raw_data[14]; // 6 Accel, 2 Temp, 6 Gyro

    if (g_mpu_data.sensor_status == 1) {
        return 1; // Sensor previously failed init
    }

    // Read 14 bytes in a burst starting from ACCEL_XOUT_H (0x3B)
    if (HAL_I2C_Mem_Read(&hi2c1, MPU6050_ADDR, MPU_ACCEL_XOUT_H_REG, 1, raw_data, 14, 100) != HAL_OK) {
        g_mpu_data.sensor_status = 1; // Mark as failed
        return 1; // I2C communication error
    }

    // --- 1. Convert Raw Data (High byte first) ---
    // Accel X, Y, Z (Indices 0, 2, 4 in raw_data)
    g_mpu_data.accel_raw[0] = (int16_t)(raw_data[0] << 8 | raw_data[1]);
    g_mpu_data.accel_raw[1] = (int16_t)(raw_data[2] << 8 | raw_data[3]);
    g_mpu_data.accel_raw[2] = (int16_t)(raw_data[4] << 8 | raw_data[5]);

    // Gyro X, Y, Z (Indices 8, 10, 12 in raw_data)
    g_mpu_data.gyro_raw[0] = (int16_t)(raw_data[8] << 8 | raw_data[9]);
    g_mpu_data.gyro_raw[1] = (int16_t)(raw_data[10] << 8 | raw_data[11]);
    g_mpu_data.gyro_raw[2] = (int16_t)(raw_data[12] << 8 | raw_data[13]);

    // --- 2. Apply Filtering and Scale Conversion ---
    for (int i = 0; i < 3; i++) {
        // Calculate the raw floating point value (x[n])
        float accel_raw_g = (float)g_mpu_data.accel_raw[i] / ACCEL_SCALE_FACTOR;
        float gyro_raw_dps = (float)g_mpu_data.gyro_raw[i] / GYRO_SCALE_FACTOR;

        // Apply IIR Filter: y[n] = y[n-1] + alpha * (x[n] - y[n-1])
        apply_iir_filter(accel_raw_g, &g_mpu_data.accel_g[i], MPU_IIR_ALPHA);
        apply_iir_filter(gyro_raw_dps, &g_mpu_data.gyro_dps[i], MPU_IIR_ALPHA);
    }

    return 0; // Success
}

/**
 * @brief Prints the filtered MPU6050 data over UART.
 */
void mpu6050_print_filtered(void) {
    if (g_mpu_data.sensor_status != 0) {
        printf("ERROR: MPU6050 sensor unavailable.\r\n");
        return;
    }

    printf("MPU6050 Filtered Data:\r\n");
    printf("  Accel (g): X: %.3f, Y: %.3f, Z: %.3f\r\n",
           g_mpu_data.accel_g[0], g_mpu_data.accel_g[1], g_mpu_data.accel_g[2]);
    printf("  Gyro (dps): X: %.3f, Y: %.3f, Z: %.3f\r\n",
           g_mpu_data.gyro_dps[0], g_mpu_data.gyro_dps[1], g_mpu_data.gyro_dps[2]);
}
