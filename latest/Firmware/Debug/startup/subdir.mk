################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../startup/startup_stm32l432xx.S 

OBJS += \
./startup/startup_stm32l432xx.o 

S_UPPER_DEPS += \
./startup/startup_stm32l432xx.d 


# Each subdirectory must supply rules for building sources it contributes
startup/%.o: ../startup/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU Assembler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -flto -Wall  -g -x assembler-with-cpp -I"/home/peter/ais_transponder/Drivers/CMSIS/Include" -I"/home/peter/ais_transponder/Drivers/CMSIS/Device/ST/STM32L4xx/Include" -I"/home/peter/ais_transponder/Drivers/STM32L4xx_HAL_Driver/Inc" -I"/home/peter/ais_transponder/Inc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


