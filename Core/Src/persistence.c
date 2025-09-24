#include "main.h" // For HAL access and RTC handle (hrtc)
#include "persistence.h"

// Global variable to hold the current wake count and timing
volatile uint32_t g_wake_count;
volatile uint32_t g_sleep_interval_s;
volatile uint32_t g_awake_interval_s;

/**
 * @brief Reads a 32-bit value from a specified RTC Backup Register.
 */
uint32_t read_backup_register(uint32_t bkp_reg) {
    // Note: The RTC handle 'hrtc' is defined in main.c by CubeMX.
    return HAL_RTCEx_BKUPRead(&hrtc, bkp_reg);
}

/**
 * @brief Writes a 32-bit value to a specified RTC Backup Register.
 */
void write_backup_register(uint32_t bkp_reg, uint32_t value) {
    // 1. Enable access to the Backup Domain (CRITICAL STEP)
    HAL_PWR_EnableBkUpAccess();

    // 2. Write the data
    HAL_RTCEx_BKUPWrite(&hrtc, bkp_reg, value);

    // 3. Optional: Disable access to save power/prevent accidental write
    HAL_PWR_DisableBkUpAccess();
}

/**
 * @brief Initializes the persistent variables on boot.
 */
void persistence_init(void) {
    uint32_t marker = read_backup_register(BKP_WAKE_COUNT_REG);

    if (marker == INITIAL_BOOT_MARKER) {
        // Not a first boot: Load the persisted values
        g_wake_count       = read_backup_register(BKP_WAKE_COUNT_REG);
        g_sleep_interval_s = read_backup_register(BKP_SLEEP_TIME_REG);
        g_awake_interval_s = read_backup_register(BKP_AWAKE_TIME_REG);

        // If timing was not set, use default (10s)
        if (g_sleep_interval_s == 0) {
            g_sleep_interval_s = 10;
            g_awake_interval_s = 10;
        }

    } else {
        // First boot: Initialize to defaults and set the marker
        g_wake_count       = 0;
        g_sleep_interval_s = 10;
        g_awake_interval_s = 10;

        write_backup_register(BKP_WAKE_COUNT_REG, INITIAL_BOOT_MARKER); // Use marker for initial save
    }
}

/**
 * @brief Increments the wake count and persists the new value.
 */
void increment_wake_count(void) {
    g_wake_count++;
    write_backup_register(BKP_WAKE_COUNT_REG, g_wake_count);
}
