# External GPS Patch (extgps)

Adds an opt-in feature to the MAIANA Transponder firmware that allows feeding
GPS NMEA sentences from an external source (Signal K server, chartplotter,
multiplexer) over USART1, instead of (or in addition to) the internal u-blox
GPS module.

**Default: OFF.** Enabling is fully runtime-configurable with no firmware
recompile.

## Why

The internal u-blox GPS module needs sky visibility. If the Maiana is mounted
indoors (cabin, electronics locker), the internal GPS may struggle to get a
fix, which prevents AIS transmission (SOTDMA timing requires GPS).

This patch lets you bypass the internal GPS and source position/time from a
better-placed receiver — either a dedicated GPS antenna at deck level or via
NMEA0183/Signal K from your chartplotter.

## How it works

| State | Internal GPS (USART2) | USART1 NMEA |
|-------|------------------------|--------------|
| `extgps off` (default) | Powered + parsed | Treated as commands only (existing behavior) |
| `extgps on`  | **Powered off** (`bsp_gnss_off`) | Lines starting with `$GP*` / `$GN*` / `$GL*` routed to GPS parser; everything else still treated as commands |

Setting persists in flash (uses `mFlags` config page, bit 1 of `flags[0]`),
survives reboots and DFU updates.

## New terminal commands

```
extgps on    -> enable feature, power down internal u-blox, persist
extgps off   -> disable feature, re-power internal u-blox, persist
extgps?      -> report status as $PAIEGPS,<0|1>*<csum>
```

## Files modified

| File | Change |
|------|--------|
| `Inc/Configuration.hpp`   | +4 method declarations (`isExtGPSEnabled`, `enable/disable/reportExtGPS`) |
| `Src/Configuration.cpp`   | +4 method impls; `enableTX/disableTX` now preserve other flag bits |
| `Src/DataTerminal.cpp`    | `termInputCB` routes `$GP*/$GN*/$GL*` to GPS event when extgps is on |
| `Src/CommandProcessor.cpp`| 3 new command branches |
| `Src/main.cpp`            | At boot, skip `GPS::enable()` if extgps was previously enabled |

## Build (Linux / OpenPlotter)

```bash
sudo apt install gcc-arm-none-eabi stm32flash make git
git clone https://github.com/<your-fork>/maiana.git
cd maiana/latest/Firmware/Transponder
make                          # default: STM32L432, BSP rev 11.3, no bootloader
```

For other variants:
```bash
make MCU=STM32L412xx           # 64k flash chip
make BSP=bsp_11_0              # older board revision
make BOOTLOADER=1              # link with VECT_TAB_OFFSET (bootloader image)
```

Output: `build/transponder.bin` (~80–110 KB depending on build config).

## Flash via USART1 ROM bootloader (no ST-Link needed)

The Maiana firmware exposes a `dfu` command that triggers a software jump to
the STM32 system bootloader at `0x1FFF0000`, which listens for new firmware
on USART1 (PA9 TX / PA10 RX).

Wiring: connect a 3.3 V TTL USB-Serial adapter (FT232, CH340, CP2102) to the
host UART pins of the Maiana.

```
USB-Serial TX  -> Maiana USART1 RX  (pin PA10)
USB-Serial RX  -> Maiana USART1 TX  (pin PA9)
USB-Serial GND -> Maiana GND
```

Then from OpenPlotter or any Linux box:
```bash
make dfu PORT=/dev/ttyUSB0
```

The `dfu` target sends the `dfu` command on the host UART at 38400 baud,
waits for the MCU to reset into bootloader, then flashes the new firmware
at 115200 baud via `stm32flash`.

If the firmware is wedged or the soft-jump fails, you can still flash the
old way:
- pull **BOOT0** pin high
- reset the MCU
- run `make flash PORT=/dev/ttyUSB0`
- pull BOOT0 low and reset again

## Enable the feature in production

After flashing, send:
```
extgps on
```
through the same USART1 terminal. The Maiana powers down the internal GPS
and starts accepting NMEA on USART1. Feed it from your Signal K server with
the `signalk-to-nmea0183` plugin (configure baud = 38400) or any NMEA0183
multiplexer.

To revert:
```
extgps off
```

Status query:
```
extgps?
```

## Compatibility

- Existing terminal commands are unchanged. Lines that don't start with
  `$GP/$GN/$GL` go to the command parser as before.
- `tx on/off` no longer clobbers other flags (now bit-precise).
- All flash data structures are unchanged — no factory reset required when
  upgrading from stock firmware.

## License

GPL v3+ (same as upstream MAIANA firmware).
