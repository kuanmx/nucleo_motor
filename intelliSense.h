#ifndef INTELLISENSE_H
#define INTELLISENSE_H

#ifdef __INTELLISENSE__

#define __MBED__ 1 == 1
#define DEVICE_I2CSLAVE 1 == 1
#define __FPU_PRESENT 1 == 1
#define DEVICE_PORTOUT 1 == 1
#define USBHOST_OTHER
#define DEVICE_PORTINOUT 1 == 1
#define TARGET_RTOS_M4_M7
#define MBED_BUILD_TIMESTAMP 1 == 1520579178.68
#define DEVICE_LOWPOWERTIMER 1 == 1
#define DEVICE_RTC 1 == 1
#define TOOLCHAIN_object
#define DEVICE_SERIAL_ASYNCH 1 == 1
#define TARGET_STM32F4
#define __CMSIS_RTOS
#define __CORTEX_M4
#define TOOLCHAIN_GCC
#define DEVICE_I2C_ASYNCH 1 == 1
#define TARGET_CORTEX_M
#define TARGET_LIKE_CORTEX_M4
#define TARGET_M4
#define TARGET_UVISOR_UNSUPPORTED
#define DEVICE_SPI_ASYNCH 1 == 1
#define TARGET_STM32F411xE
#define DEVICE_SERIAL 1 == 1
#define DEVICE_INTERRUPTIN 1 == 1
#define TARGET_CORTEX
#define DEVICE_I2C 1 == 1
#define TRANSACTION_QUEUE_SIZE_SPI 1 == 1 // ori value: 2
#define TARGET_NUCLEO_F411RE
#define DEVICE_STDIO_MESSAGES 1 == 1
#define TARGET_FF_MORPHO
#define TARGET_FAMILY_STM32
#define TARGET_FF_ARDUINO
#define DEVICE_PORTIN 1 == 1
#define TARGET_RELEASE
#define TARGET_STM
#define DEVICE_SERIAL_FC 1 == 1
#define TARGET_LIKE_MBED
#define __MBED_CMSIS_RTOS_CM
#define DEVICE_SLEEP 1 == 1
#define TOOLCHAIN_GCC_ARM
#define DEVICE_SPI 1 == 1
#define USB_STM_HAL
#define DEVICE_SPISLAVE 1 == 1
#define DEVICE_ANALOGIN 1 == 1
#define DEVICE_PWMOUT 1 == 1
#define DEVICE_FLASH 1 == 1
#define TARGET_STM32F411RE
#define ARM_MATH_CM4

// Configuration parameters
#define MBED_CONF_PLATFORM_FORCE_NON_COPYABLE_ERROR 0                            // set by library:platform
#define MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE 115200                       // set by library:platform
#define MBED_CONF_PLATFORM_STDIO_CONVERT_NEWLINES   0                            // set by library:platform
#define MBED_CONF_TARGET_LSE_AVAILABLE              1                            // set by target:FAMILY_STM32
#define MBED_CONF_PLATFORM_STDIO_BAUD_RATE          115200                       // set by library:platform
#define CLOCK_SOURCE                                USE_PLL_HSE_EXTC|USE_PLL_HSI // set by target:NUCLEO_F411RE
#define CLOCK_SOURCE_USB                            0                            // set by target:NUCLEO_F411RE
#define MBED_CONF_PLATFORM_STDIO_FLUSH_AT_EXIT      1                            // set by library:platform

#endif // __INTELLISENSE__
#endif