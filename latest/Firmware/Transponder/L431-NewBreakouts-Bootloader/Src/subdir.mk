################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Src/AISMessages.cpp \
../Src/ChannelManager.cpp \
../Src/CommandProcessor.cpp \
../Src/Configuration.cpp \
../Src/DataTerminal.cpp \
../Src/EventQueue.cpp \
../Src/Events.cpp \
../Src/GPS.cpp \
../Src/LEDManager.cpp \
../Src/NMEAEncoder.cpp \
../Src/NMEASentence.cpp \
../Src/NoiseFloorDetector.cpp \
../Src/RFIC.cpp \
../Src/RXPacket.cpp \
../Src/RXPacketProcessor.cpp \
../Src/RadioManager.cpp \
../Src/Receiver.cpp \
../Src/TXPacket.cpp \
../Src/TXScheduler.cpp \
../Src/Transceiver.cpp \
../Src/Utils.cpp \
../Src/main.cpp \
../Src/printf_serial.cpp \
../Src/si4460.cpp \
../Src/si4463.cpp \
../Src/si4467.cpp 

C_SRCS += \
../Src/stm32l4xx_it.c \
../Src/system_stm32l4xx.c 

C_DEPS += \
./Src/stm32l4xx_it.d \
./Src/system_stm32l4xx.d 

OBJS += \
./Src/AISMessages.o \
./Src/ChannelManager.o \
./Src/CommandProcessor.o \
./Src/Configuration.o \
./Src/DataTerminal.o \
./Src/EventQueue.o \
./Src/Events.o \
./Src/GPS.o \
./Src/LEDManager.o \
./Src/NMEAEncoder.o \
./Src/NMEASentence.o \
./Src/NoiseFloorDetector.o \
./Src/RFIC.o \
./Src/RXPacket.o \
./Src/RXPacketProcessor.o \
./Src/RadioManager.o \
./Src/Receiver.o \
./Src/TXPacket.o \
./Src/TXScheduler.o \
./Src/Transceiver.o \
./Src/Utils.o \
./Src/main.o \
./Src/printf_serial.o \
./Src/si4460.o \
./Src/si4463.o \
./Src/si4467.o \
./Src/stm32l4xx_it.o \
./Src/system_stm32l4xx.o 

CPP_DEPS += \
./Src/AISMessages.d \
./Src/ChannelManager.d \
./Src/CommandProcessor.d \
./Src/Configuration.d \
./Src/DataTerminal.d \
./Src/EventQueue.d \
./Src/Events.d \
./Src/GPS.d \
./Src/LEDManager.d \
./Src/NMEAEncoder.d \
./Src/NMEASentence.d \
./Src/NoiseFloorDetector.d \
./Src/RFIC.d \
./Src/RXPacket.d \
./Src/RXPacketProcessor.d \
./Src/RadioManager.d \
./Src/Receiver.d \
./Src/TXPacket.d \
./Src/TXScheduler.d \
./Src/Transceiver.d \
./Src/Utils.d \
./Src/main.d \
./Src/printf_serial.d \
./Src/si4460.d \
./Src/si4463.d \
./Src/si4467.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.cpp Src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fsingle-precision-constant -flto -Wall -DSTM32L431xx -DVECT_TAB_OFFSET=0x4800 -DLEGACY_BREAKOUTS=0 -I"/Users/peter/github/maiana/latest/Firmware/Transponder/Inc" -I"/Users/peter/github/maiana/latest/Firmware/Transponder/Inc/bsp" -I"/Users/peter/github/maiana/latest/Firmware/Transponder/Drivers/CMSIS/Device/ST/STM32L4xx/Include" -I"/Users/peter/github/maiana/latest/Firmware/Transponder/Drivers/CMSIS/Include" -I"/Users/peter/github/maiana/latest/Firmware/Transponder/Drivers/STM32L4xx_HAL_Driver/Inc" -std=gnu++11 -fabi-version=0 -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Src/%.o: ../Src/%.c Src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fsingle-precision-constant -flto -Wall -DSTM32L431xx -DVECT_TAB_OFFSET=0x4800 -DLEGACY_BREAKOUTS=0 -I"/Users/peter/github/maiana/latest/Firmware/Transponder/Drivers/CMSIS/Include" -I"/Users/peter/github/maiana/latest/Firmware/Transponder/Inc/bsp" -I"/Users/peter/github/maiana/latest/Firmware/Transponder/Drivers/CMSIS/Device/ST/STM32L4xx/Include" -I"/Users/peter/github/maiana/latest/Firmware/Transponder/Drivers/STM32L4xx_HAL_Driver/Inc" -I"/Users/peter/github/maiana/latest/Firmware/Transponder/Inc" -std=gnu11 -Wa,-adhlns="$@.lst" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


