################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../system/src/stm32f3-stdperiph/stm32f30x_flash.c \
../system/src/stm32f3-stdperiph/stm32f30x_gpio.c \
../system/src/stm32f3-stdperiph/stm32f30x_misc.c \
../system/src/stm32f3-stdperiph/stm32f30x_rcc.c \
../system/src/stm32f3-stdperiph/stm32f30x_usart.c 

OBJS += \
./system/src/stm32f3-stdperiph/stm32f30x_flash.o \
./system/src/stm32f3-stdperiph/stm32f30x_gpio.o \
./system/src/stm32f3-stdperiph/stm32f30x_misc.o \
./system/src/stm32f3-stdperiph/stm32f30x_rcc.o \
./system/src/stm32f3-stdperiph/stm32f30x_usart.o 

C_DEPS += \
./system/src/stm32f3-stdperiph/stm32f30x_flash.d \
./system/src/stm32f3-stdperiph/stm32f30x_gpio.d \
./system/src/stm32f3-stdperiph/stm32f30x_misc.d \
./system/src/stm32f3-stdperiph/stm32f30x_rcc.d \
./system/src/stm32f3-stdperiph/stm32f30x_usart.d 


# Each subdirectory must supply rules for building sources it contributes
system/src/stm32f3-stdperiph/%.o: ../system/src/stm32f3-stdperiph/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -flto -Wall -Wextra  -g -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F30X -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f3-stdperiph" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


