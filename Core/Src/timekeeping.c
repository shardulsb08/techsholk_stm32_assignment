#include "main.h" // Assumes extern RTC_HandleTypeDef hrtc is available
#include <stdio.h> // For snprintf

// Note: Ensure the RTC handle (hrtc) is declared as extern if this is a separate file.

/**
  * @brief Retrieves the current date and time from the RTC and formats it into a string.
  * @param buffer: Pointer to a character array (must be at least 20 bytes long)
  */
void get_current_timestamp_string(char *buffer) {
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    // Note: HAL requires calling HAL_RTC_GetTime() first, then HAL_RTC_GetDate()
    // to ensure consistency across the shadow registers.
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    // Format the output string: YYYY-MM-DD HH:MM:SS
    // RTC_Date.Year is a 2-digit BCD/BIN year (e.g., 25 for 2025)
    snprintf(buffer, 30, "%04hu-%02hu-%02hu %02hu:%02hu:%02hu", 
             (uint16_t)(sDate.Year + 2000), // Convert 2-digit year to 4-digit
             sDate.Month, 
             sDate.Date, 
             sTime.Hours, 
             sTime.Minutes, 
             sTime.Seconds);
}
