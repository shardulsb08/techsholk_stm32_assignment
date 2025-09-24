#define BKP_WAKE_COUNT_REG  RTC_BKP_DR0
#define BKP_SLEEP_TIME_REG  RTC_BKP_DR1
#define BKP_AWAKE_TIME_REG  RTC_BKP_DR2
#define INITIAL_BOOT_MARKER 0xCAFEFEED // A unique value to detect the first boot
extern RTC_HandleTypeDef hrtc;
