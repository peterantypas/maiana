################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/_write.c 

CPP_SRCS += \
../src/AISMessages.cpp \
../src/ChannelManager.cpp \
../src/DataTerminal.cpp \
../src/DebugPrinter.cpp \
../src/EEPROM.cpp \
../src/EventQueue.cpp \
../src/Events.cpp \
../src/GPS.cpp \
../src/LEDManager.cpp \
../src/MenuScreens.cpp \
../src/NMEAEncoder.cpp \
../src/NMEASentence.cpp \
../src/NoiseFloorDetector.cpp \
../src/RFIC.cpp \
../src/RXPacket.cpp \
../src/RXPacketProcessor.cpp \
../src/RadioManager.cpp \
../src/Receiver.cpp \
../src/TXPacket.cpp \
../src/TXScheduler.cpp \
../src/Transceiver.cpp \
../src/Utils.cpp \
../src/main.cpp \
../src/printf2.cpp 

OBJS += \
./src/AISMessages.o \
./src/ChannelManager.o \
./src/DataTerminal.o \
./src/DebugPrinter.o \
./src/EEPROM.o \
./src/EventQueue.o \
./src/Events.o \
./src/GPS.o \
./src/LEDManager.o \
./src/MenuScreens.o \
./src/NMEAEncoder.o \
./src/NMEASentence.o \
./src/NoiseFloorDetector.o \
./src/RFIC.o \
./src/RXPacket.o \
./src/RXPacketProcessor.o \
./src/RadioManager.o \
./src/Receiver.o \
./src/TXPacket.o \
./src/TXScheduler.o \
./src/Transceiver.o \
./src/Utils.o \
./src/_write.o \
./src/main.o \
./src/printf2.o 

C_DEPS += \
./src/_write.d 

CPP_DEPS += \
./src/AISMessages.d \
./src/ChannelManager.d \
./src/DataTerminal.d \
./src/DebugPrinter.d \
./src/EEPROM.d \
./src/EventQueue.d \
./src/Events.d \
./src/GPS.d \
./src/LEDManager.d \
./src/MenuScreens.d \
./src/NMEAEncoder.d \
./src/NMEASentence.d \
./src/NoiseFloorDetector.d \
./src/RFIC.d \
./src/RXPacket.d \
./src/RXPacketProcessor.d \
./src/RadioManager.d \
./src/Receiver.d \
./src/TXPacket.d \
./src/TXScheduler.d \
./src/Transceiver.d \
./src/Utils.d \
./src/main.d \
./src/printf2.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C++ Compiler'
	arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -flto -Wall -Wextra  -g -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F30X -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f3-stdperiph" -std=gnu++11 -fabi-version=0 -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -flto -Wall -Wextra  -g -DOS_USE_TRACE_SEMIHOSTING_DEBUG -DSTM32F30X -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f3-stdperiph" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


