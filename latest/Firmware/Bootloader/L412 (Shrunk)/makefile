################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include startup/subdir.mk
-include Src/subdir.mk
-include Drivers/STM32L4xx_HAL_Driver/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := maiana-bootloader
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
maiana-bootloader.bin \

SECONDARY_SIZE += \
maiana-bootloader.siz \


# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: maiana-bootloader.elf

# Tool invocations
maiana-bootloader.elf: $(OBJS) $(USER_OBJS) makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM Cross C Linker'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -flto -Wall -T "/Users/peter/github/maiana/latest/Firmware/Bootloader/STM32L4xx_FLASH.ld" -Xlinker --gc-sections -s -Wl,-Map,"maiana-bootloader.map" --specs=nano.specs --specs=nosys.specs --specs=rdimon.specs -o "maiana-bootloader.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

maiana-bootloader.bin: maiana-bootloader.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: GNU ARM Cross Create Flash Image'
	arm-none-eabi-objcopy -O binary "maiana-bootloader.elf"  "maiana-bootloader.bin"
	@echo 'Finished building: $@'
	@echo ' '

maiana-bootloader.siz: maiana-bootloader.elf makefile objects.mk $(OPTIONAL_TOOL_DEPS)
	@echo 'Invoking: GNU ARM Cross Print Size'
	arm-none-eabi-size --format=sysv "maiana-bootloader.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS) maiana-bootloader.elf
	-@echo ' '

post-build:
	-@echo 'Generating binary and Printing size information:'
	-arm-none-eabi-objcopy -O binary "maiana-bootloader.elf" "maiana-bootloader.bin" && arm-none-eabi-size "maiana-bootloader.elf"
	-@echo ' '

.PHONY: all clean dependents main-build post-build

-include ../makefile.targets
