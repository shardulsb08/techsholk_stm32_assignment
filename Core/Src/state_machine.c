#include"state_machine.h"
#include <stdio.h>

// Define the global state and timing variables (must be 'extern' in the header)
volatile SystemState_t g_current_state = STATE_WAKE; 
// g_sleep_interval_s and g_awake_interval_s are managed by persistence.c

static uint8_t first_run = 1;
// Global tick counter for LED blink (100ms ON / 100ms OFF = 200ms cycle)
uint32_t led_toggle_tick = 0; 
extern uint32_t wake_end_tick;

/**
 * @brief Handles the LED blinking while in the WAKE state.
 */
void handle_led_blink(void) {
    if (HAL_GetTick() - led_toggle_tick >= 100) { // 100ms interval
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5); // PA5 is our LED
        led_toggle_tick = HAL_GetTick();
    }
}

/**
 * @brief Actions executed during the WAKE state.
 */
void handle_wake_state(void) {

    // This block executes only once per wake event (timer or button)
    if (first_run) {
        // --- Wake Event Actions (§3) ---
        increment_wake_count(); // §6: Increment and persist
        
        char timestamp[30];
        get_current_timestamp_string(timestamp); // Requires Phase 1 function
        
        printf("\r\n--- Hello! Waking up ---\r\n");
        printf("Current Time: %s\r\n", timestamp); // §1
        printf("Wake Count: %lu\r\n", g_wake_count); // §6
        
        mpu6050_read_and_filter(); // §9: Read MPU6050
        mpu6050_print_filtered();    // §9: Print filtered data (includes error check)
        
        first_run = 0;
    }

    // --- Continuous Actions ---
    handle_led_blink(); // §5: Blink LED
    // Check for serial commands (Parsing handled in Phase 4)
    // ... 
}

/**
 * @brief Prepares and enters the low-power sleep state.
 */
void enter_sleep(void) {
    // 1. Set the state
    g_current_state = STATE_SLEEP;

    // 2. Stop LED blink and ensure LED is OFF (§5)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    
    // 3. Configure RTC Wakeup Timer for sleep interval (§3)
    // The RTC clock source is LSE (32768 Hz). We need to calculate the counter.
    // L072 uses RTCCLK/16, so Wakeup Period = (Wakeup_Counter + 1) * (16 / 32768)
    // Wakeup_Counter = (Wakeup_Time * 32768 / 16) - 1
    // For T=10s: Counter = (10 * 2048) - 1 = 20479 (0x4FFF)
    
    uint32_t counter = (g_sleep_interval_s * 2048) - 1; // 2048 = 32768 / 16
    
    // Set the counter and enable the interrupt
    HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, counter, RTC_WAKEUPCLOCK_RTCCLK_DIV16);

    // 4. Suspend the SysTick timer (required for deep sleep)
    HAL_SuspendTick(); 

    // 5. Enter Low Power STOP Mode 2 (retains RAM, can be woken by RTC/EXTI)
    // Clear any pending wakeup flags, ensuring a clean entry into sleep
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU); // Clear WAKEUP Flag before sleep
    // We use PWR_LOWPOWERREGULATOR_ON to achieve low consumption in Stop mode
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    // --- MCU Wakes Up Here (Execution resumes after interrupt) ---

    // 6. Resume the System Tick (Critical for HAL_Delay/HAL_GetTick to work)
    HAL_ResumeTick();
    
    // 7. Reset the Wake timer expiry for the WAKE state
    wake_end_tick = HAL_GetTick() + (g_awake_interval_s * 1000);
    
    // 8. Re-initialize the MPU6050 (some peripherals can lose state)
    // mpu6050_init(); // Optional, but good practice if I2C clock is disabled in Stop mode.

    // 9. Mark for first run actions
    first_run = 1;
    g_current_state = STATE_WAKE;
}
