## Pre-built binaries

This directory contains binaries for boards 11.3 and newer *purchased through the OpenMarine store*. **If you do not use OpenPlotter 
or you are unsure whether these apply to your board, please start a ticket or a discussion**. Do NOT attempt to flash these with an SWD programmer. They are not linked for the top of flash (0x08000000). Furthermore, they *must* be used in conjunction with the MAIANA bootloader and should only be written via the UART. The bootloader will checksum them, write their metadata in a dedicated flash page, and only then will it execute them.

### How to select the right firmware

Launch the MAIANA app and connect to the transponder. You may need to disable / disconnect MAIANA from Signal K first.
Make a note of:

- Your board's hardware revision (example: 11.6.0)
- Your board's MCU model (example: STM32L422)
- Your board's firmware revision (example: 4.0.0)

Locate the appropriate binary in this directory. Ignore the last digit of your board's revision -- it doesn't matter. So if you have board 11.3.0 with an STM32L422 processor, the right binary is maiana-stm32l422-hw11.3-fwXXX.bin where XXX is the latest revision you see here. If you already have this firmware on your board, there is no update.


## Firmware update history

### Version 4.4.1

This release fixes a bug introduced in 4.3.0, where boards with the L412 processor do not report the MCU in the CLI.

### Version 4.4.0

This release added a CLI for trimming the crystals during manufacturing. This will be necessary going forward.

### Version 4.3.0

This release includes enhancements for manufacturing

### Version 4.1.0

This firmware release corrects the tuning of the crystal oscillator for the Si4467 chips. This applies to boards made after November 2021 which contain the following
crystal:

![IMG_1673](https://user-images.githubusercontent.com/1565933/156231414-5624cf9b-d8ff-4dc6-9451-eac7009387b8.jpg)

The tuning changed by 5 KHz when the design had to migrate to this crystal due to supply problems. See issue [#103](https://github.com/peterantypas/maiana/issues/103)

### Version 4.0.0

This firmware release makes use of the new MAIANA bootloader.
