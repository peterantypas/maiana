################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/stm32l4xx_hal_msp.c \
../Src/stm32l4xx_it.c \
../Src/system_stm32l4xx.c 

CPP_SRCS += \
../Src/AISMessages.cpp \
../Src/ChannelManager.cpp \
../Src/CommandProcessor.cpp \
../Src/Configuration.cpp \
../Src/DataTerminal.cpp \
../Src/EventQueue.cpp \
../Src/Events.cpp \
../Src/GPS.cpp \
../Src/NMEAEncoder.cpp \
../Src/NMEASentence.cpp \
../Src/NoiseFloorDetector.cpp \
../Src/RFIC.cpp \
../Src/RXPacket.cpp \
../Src/RXPacketProcessor.cpp \
../Src/RadioManager.cpp \
../Src/Receiver.cpp \
../Src/SystickTimer.cpp \
../Src/TXPacket.cpp \
../Src/TXScheduler.cpp \
../Src/Transceiver.cpp \
../Src/Utils.cpp \
../Src/main.cpp \
../Src/printf_serial.cpp 

OBJS += \
./Src/AISMessages.o \
./Src/ChannelManager.o \
./Src/CommandProcessor.o \
./Src/Configuration.o \
./Src/DataTerminal.o \
./Src/EventQueue.o \
./Src/Events.o \
./Src/GPS.o \
./Src/NMEAEncoder.o \
./Src/NMEASentence.o \
./Src/NoiseFloorDetector.o \
./Src/RFIC.o \
./Src/RXPacket.o \
./Src/RXPacketProcessor.o \
./Src/RadioManager.o \
./Src/Receiver.o \
./Src/SystickTimer.o \
./Src/TXPacket.o \
./Src/TXScheduler.o \
./Src/Transceiver.o \
./Src/Utils.o \
./Src/main.o \
./Src/printf_serial.o \
./Src/stm32l4xx_hal_msp.o \
./Src/stm32l4xx_it.o \
./Src/system_stm32l4xx.o 

C_DEPS += \
./Src/stm32l4xx_hal_msp.d \
./Src/stm32l4xx_it.d \
./Src/system_stm32l4xx.d 

CPP_DEPS += \
./Src/AISMessages.d \
./Src/ChannelManager.d \
./Src/CommandProcessor.d \
./Src/Configuration.d \
./Src/DataTerminal.d \
./Src/EventQueue.d \
./Src/Events.d \
./Src/GPS.d \
./Src/NMEAEncoder.d \
./Src/NMEASentence.d \
./Src/NoiseFloorDetector.d \
./Src/RFIC.d \
./Src/RXPacket.d \
./Src/RXPacketProcessor.d \
./Src/RadioManager.d \
./Src/Receiver.d \
./Src/SystickTimer.d \
./Src/TXPacket.d \
./Src/TXScheduler.d \
./Src/Transceiver.d \
./Src/Utils.d \
./Src/main.d \
./Src/printf_serial.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -flto -Wall  -g -I"/home/peter/ais_transponder/Inc" -I"/home/peter/ais_transponder/Inc/bsp" -I"/home/peter/ais_transponder/Drivers/CMSIS/Device/ST/STM32L4xx/Include" -I"/home/peter/ais_transponder/Drivers/CMSIS/Include" -I"/home/peter/ais_transponder/Drivers/STM32L4xx_HAL_Driver/Inc" -std=gnu++11 -fabi-version=0 -fno-exceptions -fno-rtti -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -flto -Wall  -g -I"/home/peter/ais_transponder/Drivers/CMSIS/Device/ST/STM32L4xx/Include" -I"/home/peter/ais_transponder/Inc/bsp" -I"/home/peter/ais_transponder/Drivers/CMSIS/Include" -I"/home/peter/ais_transponder/Inc" -I"/home/peter/ais_transponder/Drivers/STM32L4xx_HAL_Driver/Inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


