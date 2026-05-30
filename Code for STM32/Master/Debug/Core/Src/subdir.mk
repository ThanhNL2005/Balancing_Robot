################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/CAN.c \
../Core/Src/ESP32.c \
../Core/Src/HWT906.c \
../Core/Src/RS485.c \
../Core/Src/SIN_COS.c \
../Core/Src/SMC.c \
../Core/Src/TIMER.c \
../Core/Src/UART.c \
../Core/Src/main.c \
../Core/Src/stm32h7xx_hal_msp.c \
../Core/Src/stm32h7xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/system_stm32h7xx.c 

OBJS += \
./Core/Src/CAN.o \
./Core/Src/ESP32.o \
./Core/Src/HWT906.o \
./Core/Src/RS485.o \
./Core/Src/SIN_COS.o \
./Core/Src/SMC.o \
./Core/Src/TIMER.o \
./Core/Src/UART.o \
./Core/Src/main.o \
./Core/Src/stm32h7xx_hal_msp.o \
./Core/Src/stm32h7xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/system_stm32h7xx.o 

C_DEPS += \
./Core/Src/CAN.d \
./Core/Src/ESP32.d \
./Core/Src/HWT906.d \
./Core/Src/RS485.d \
./Core/Src/SIN_COS.d \
./Core/Src/SMC.d \
./Core/Src/TIMER.d \
./Core/Src/UART.d \
./Core/Src/main.d \
./Core/Src/stm32h7xx_hal_msp.d \
./Core/Src/stm32h7xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/system_stm32h7xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H750xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/CAN.cyclo ./Core/Src/CAN.d ./Core/Src/CAN.o ./Core/Src/CAN.su ./Core/Src/ESP32.cyclo ./Core/Src/ESP32.d ./Core/Src/ESP32.o ./Core/Src/ESP32.su ./Core/Src/HWT906.cyclo ./Core/Src/HWT906.d ./Core/Src/HWT906.o ./Core/Src/HWT906.su ./Core/Src/RS485.cyclo ./Core/Src/RS485.d ./Core/Src/RS485.o ./Core/Src/RS485.su ./Core/Src/SIN_COS.cyclo ./Core/Src/SIN_COS.d ./Core/Src/SIN_COS.o ./Core/Src/SIN_COS.su ./Core/Src/SMC.cyclo ./Core/Src/SMC.d ./Core/Src/SMC.o ./Core/Src/SMC.su ./Core/Src/TIMER.cyclo ./Core/Src/TIMER.d ./Core/Src/TIMER.o ./Core/Src/TIMER.su ./Core/Src/UART.cyclo ./Core/Src/UART.d ./Core/Src/UART.o ./Core/Src/UART.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32h7xx_hal_msp.cyclo ./Core/Src/stm32h7xx_hal_msp.d ./Core/Src/stm32h7xx_hal_msp.o ./Core/Src/stm32h7xx_hal_msp.su ./Core/Src/stm32h7xx_it.cyclo ./Core/Src/stm32h7xx_it.d ./Core/Src/stm32h7xx_it.o ./Core/Src/stm32h7xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/system_stm32h7xx.cyclo ./Core/Src/system_stm32h7xx.d ./Core/Src/system_stm32h7xx.o ./Core/Src/system_stm32h7xx.su

.PHONY: clean-Core-2f-Src

