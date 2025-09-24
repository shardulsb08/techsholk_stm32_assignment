#ifndef __STATE_MACHINE_H
#define __STATE_MACHINE_H

#include <stdint.h>
#include "stm32l0xx_hal.h"
#include "persistence.h"
#include "mpu6050.h"
#include "timekeeping.h"

// --- System States ---
typedef enum {
    STATE_WAKE,
    STATE_SLEEP
} SystemState_t;

extern volatile SystemState_t g_current_state;

// --- Timing Variables (Seconds, loaded from persistence) ---
extern volatile uint32_t g_sleep_interval_s; // Default 10s
extern volatile uint32_t g_awake_interval_s; // Default 10s

// --- Global Function Prototypes ---
void enter_sleep(void);
void handle_wake_state(void);

#endif // __STATE_MACHINE_H
