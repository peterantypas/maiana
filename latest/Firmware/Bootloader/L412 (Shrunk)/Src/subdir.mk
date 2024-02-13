################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/crc32.c \
../Src/fw_update.c \
../Src/main.c \
../Src/printf2.c \
../Src/stm32l4xx_hal_msp.c \
../Src/stm32l4xx_it.c \
../Src/system_stm32l4xx.c 

OBJS += \
./Src/crc32.o \
./Src/fw_update.o \
./Src/main.o \
./Src/printf2.o \
./Src/stm32l4xx_hal_msp.o \
./Src/stm32l4xx_it.o \
./Src/system_stm32l4xx.o 

C_DEPS += \
./Src/crc32.d \
./Src/fw_update.d \
./Src/main.d \
./Src/printf2.d \
./Src/stm32l4xx_hal_msp.d \
./Src/stm32l4xx_it.d \
./Src/system_stm32l4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c Src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU ARM Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -flto -Wall -DSTM32L412xx -D__SHRINK_TO_64K__=1 -I"/Users/peter/github/maiana/latest/Firmware/Bootloader/Inc" -I"/Users/peter/github/maiana/latest/Firmware/Bootloader/Drivers/STM32L4xx_HAL_Driver/Inc" -I"/Users/peter/github/maiana/latest/Firmware/Bootloader/Drivers/CMSIS/Include" -I"/Users/peter/github/maiana/latest/Firmware/Bootloader/Drivers/CMSIS/Device/ST/STM32L4xx/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


