# =============================================================================
#  MAIANA Transponder firmware — CLI build (Linux / OpenPlotter / macOS)
#  Replaces Eclipse builder with a plain make + arm-none-eabi-gcc workflow.
#
#  Usage:
#    make                              # default: STM32L432, BSP rev 11.3, no bootloader
#    make MCU=STM32L412xx              # build for L412
#    make BSP=bsp_11_0                 # select BSP source
#    make BOOTLOADER=1                 # link with VECT_TAB_OFFSET (bootloader build)
#    make clean
#    make flash PORT=/dev/ttyUSB0      # via stm32flash over USART1 ROM bootloader
#    make dfu PORT=/dev/ttyUSB0        # send 'dfu' command, then flash
#
#  Toolchain: arm-none-eabi-gcc (Debian/Ubuntu: apt install gcc-arm-none-eabi)
# =============================================================================

# ---- Project ----
TARGET           := transponder
BUILD_DIR        := build

# ---- Configurable ----
MCU              ?= STM32L432xx        # STM32L412xx / STM32L422xx / STM32L432xx
BSP              ?= bsp_11_3           # bsp_9_3 / bsp_10_0 / bsp_10_1 / bsp_10_5 / bsp_10_9 / bsp_11_0 / bsp_11_3
BOOTLOADER       ?= 0                  # 1 = link at VECT_TAB_OFFSET for bootloader

# ---- Toolchain ----
PREFIX           := arm-none-eabi-
CC               := $(PREFIX)gcc
CXX              := $(PREFIX)g++
AS               := $(PREFIX)gcc -x assembler-with-cpp
OBJCOPY          := $(PREFIX)objcopy
SIZE             := $(PREFIX)size

# ---- MCU flags ----
MCUFLAGS         := -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

# ---- Optimization ----
OPT              := -Os -flto -ffunction-sections -fdata-sections \
                    -fsigned-char -ffreestanding -fsingle-precision-constant

# ---- Defines ----
DEFS             := -D$(MCU) -DUSE_HAL_DRIVER

ifeq ($(BOOTLOADER),1)
  DEFS           += -DVECT_TAB_OFFSET=0x4000
  ifeq ($(MCU),STM32L412xx)
    LDSCRIPT     := STM32L412_64k-Bootloader.ld
  else
    LDSCRIPT     := STM32L4xx_FLASH-Bootloader.ld
  endif
else
  ifeq ($(MCU),STM32L412xx)
    LDSCRIPT     := STM32L412_64k.ld
  else
    LDSCRIPT     := STM32L4xx_FLASH.ld
  endif
endif

# ---- Includes ----
INCLUDES         := \
  -IInc \
  -IInc/bsp \
  -IDrivers/CMSIS/Device/ST/STM32L4xx/Include \
  -IDrivers/CMSIS/Include \
  -IDrivers/STM32L4xx_HAL_Driver/Inc \
  -IFreeRTOS/include \
  -IFreeRTOS/portable/GCC/ARM_CM4F

# ---- Sources ----
# All firmware C++ sources
CPP_SRCS         := $(wildcard Src/*.cpp)
# Single BSP variant
CPP_SRCS         += Src/bsp/$(BSP).cpp
# C sources (firmware + HAL drivers)
C_SRCS           := $(wildcard Src/*.c) \
                    $(wildcard Drivers/STM32L4xx_HAL_Driver/Src/*.c)
# FreeRTOS sources (if any used — included for completeness)
C_SRCS           += $(wildcard FreeRTOS/*.c) \
                    $(wildcard FreeRTOS/portable/GCC/ARM_CM4F/*.c) \
                    $(wildcard FreeRTOS/portable/MemMang/heap_4.c)
# Startup
ifeq ($(MCU),STM32L412xx)
  ASM_SRCS       := startup/startup_stm32l412xx.S
else
  ASM_SRCS       := startup/startup_stm32l432xx.S
endif

# Filter out _template files commonly present in HAL distros
C_SRCS           := $(filter-out %_template.c %_msp_template.c, $(C_SRCS))

# ---- Object files ----
CPP_OBJS         := $(CPP_SRCS:%.cpp=$(BUILD_DIR)/%.o)
C_OBJS           := $(C_SRCS:%.c=$(BUILD_DIR)/%.o)
ASM_OBJS         := $(ASM_SRCS:%.S=$(BUILD_DIR)/%.o)
OBJS             := $(CPP_OBJS) $(C_OBJS) $(ASM_OBJS)

# ---- Compiler flags ----
CFLAGS           := $(MCUFLAGS) $(OPT) $(DEFS) $(INCLUDES) -Wall -fmessage-length=0
CXXFLAGS         := $(CFLAGS) -fno-exceptions -fno-rtti -std=gnu++14
ASFLAGS          := $(MCUFLAGS) -Wall

LDFLAGS          := $(MCUFLAGS) -T$(LDSCRIPT) --specs=nano.specs \
                    -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
                    -lm -lstdc++

# ---- Targets ----
.PHONY: all clean flash dfu size

all: $(BUILD_DIR)/$(TARGET).bin size

$(BUILD_DIR)/$(TARGET).elf: $(OBJS) $(LDSCRIPT)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# C++ rule
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# C rule
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

# Assembler rule
$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) -c $< -o $@

size: $(BUILD_DIR)/$(TARGET).elf
	@$(SIZE) $<

clean:
	@rm -rf $(BUILD_DIR)

# ---- Flash via STM32 ROM bootloader on USART1 ----
PORT             ?= /dev/ttyUSB0
BAUD             ?= 115200

flash: $(BUILD_DIR)/$(TARGET).bin
	@echo ">>> Make sure the device is in DFU mode (send 'dfu' command via terminal first, or pull BOOT0 high + reset)"
	stm32flash -b $(BAUD) -w $< -v $(PORT)

# Send 'dfu' command via the same UART, then flash. Requires the firmware to
# already be running and listening on USART1 at 38400 baud.
dfu: $(BUILD_DIR)/$(TARGET).bin
	@echo ">>> Sending 'dfu' command at 38400 baud to $(PORT)..."
	@stty -F $(PORT) 38400 cs8 -cstopb -parenb -echo raw 2>/dev/null || true
	@printf 'dfu\r\n' > $(PORT)
	@sleep 1
	@echo ">>> Flashing via ROM bootloader at $(BAUD) baud..."
	stm32flash -b $(BAUD) -w $< -v -g 0x08000000 $(PORT)
