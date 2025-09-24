#ifndef __UART_COMMAND_H
#define __UART_COMMAND_H

#include "main.h"

#define MAX_COMMAND_LENGTH 30
#define RX_BUFFER_SIZE     64

extern volatile uint8_t g_rx_byte;
extern char g_command_buffer[MAX_COMMAND_LENGTH + 1];
extern volatile uint8_t g_command_ready_flag;

// Function Prototypes
void command_init(void);
void handle_serial_input(void);

#endif // __UART_COMMAND_H
