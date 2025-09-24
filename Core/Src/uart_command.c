#include "uart_command.h"
#include "state_machine.h" // For g_sleep_interval_s, enter_sleep, etc.
#include "persistence.h"   // For write_backup_register
#include <stdio.h>
#include <string.h>

// --- Global Variables for UART Command Handling ---

// Single byte for the interrupt handler
volatile uint8_t g_rx_byte;

// Command buffer and index
char g_command_buffer[MAX_COMMAND_LENGTH + 1];
volatile uint8_t g_command_ready_flag = 0;

// Handle for UART2 is external
extern UART_HandleTypeDef huart2;

/**
  * @brief Initializes UART reception and peripheral dependencies.
  */
void command_init(void) {
    // Start listening for the first byte, enabling interrupt mode
    HAL_UART_Receive_IT(&huart2, (uint8_t*)&g_rx_byte, 1);
}

/**
  * @brief Parses and executes the received serial command.
  */
static void parse_command(void) {
    // --- Command: sleep now (§7) ---
    if (strcmp(g_command_buffer, "sleep") == 0) {
        printf("SERIAL COMMAND: Entering sleep immediately.\r\n");
        // Exit the current run cycle and enter low power mode
        enter_sleep();
        return;
    }

    // --- Command: set timing (stime:<seconds>) (§8) ---
    uint32_t new_interval_s;
    int items_read = sscanf(g_command_buffer, "stime:%lu", &new_interval_s);

    if (items_read == 1) {
        // Validate range: 1–86400 seconds
        if (new_interval_s >= 1 && new_interval_s <= 86400) {

            // 1. Update global variables
            g_sleep_interval_s = new_interval_s;
            g_awake_interval_s = new_interval_s;

            // 2. Persistently save the new value (§8)
            write_backup_register(BKP_SLEEP_TIME_REG, g_sleep_interval_s);
            write_backup_register(BKP_AWAKE_TIME_REG, g_awake_interval_s);

            printf("SERIAL COMMAND: Set cycle interval to %lu seconds. (New value saved)\r\n", new_interval_s);

            // Note: The timer for the current WAKE cycle needs to be reset
            // for the change to take effect immediately, which is handled
            // by setting the wake_end_tick in main.c

        } else {
            printf("SERIAL ERROR: Invalid interval: %lu. Range is 1-86400 seconds.\r\n", new_interval_s);
        }
        return;
    }

    // --- Unknown Command ---
    printf("SERIAL ERROR: Unknown command '%s'.\r\n", g_command_buffer);
}

/**
  * @brief Checks the flag and executes the command parser if input is ready.
  */
void handle_serial_input(void) {
    if (g_command_ready_flag) {
        parse_command();
        g_command_ready_flag = 0; // Clear flag for next command
        // Note: The buffer contents are overwritten on the next successful reception.
    }
}
