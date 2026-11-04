# TDP3 Team 7: robot firmware (FRDM-KL25Z)

Interim demo: the robot drives a fixed, pre-programmed path (no line following).
Plain C for the NXP MKL25Z128 (Cortex-M0+, 48 MHz). It doesn't use Mbed (end-of-life since July 2026) or an IDE plugin. It builds with the official Arm GNU compiler, driven by `tdp.py`.

Wiring is in **[WIRING.md](WIRING.md)**.

## One-time setup (macOS, also works on Windows/Linux)

Open this folder in VSCode, open a terminal (``Ctrl+` ``) and run:

```
python3 tdp.py setup
```

This creates:
- a Python venv in `~/.tdp3/venv` (with `pyserial` for the serial monitor)
- the Arm GNU Toolchain 14.2 in `~/.tdp3/arm-gnu-toolchain` (about 150 MB download, no admin rights needed)
- the VSCode build tasks and settings in `.vscode/`, copied from `vscode-template/`. Run `python3 tdp.py vscode` to reinstall them.

Both live outside OneDrive on purpose: OneDrive doesn't sync a venv's symlinks properly, and 1 GB of compiler shouldn't sync anyway. Then reload VSCode (Cmd+Shift+P → "Reload Window"). New terminals in this folder then have the venv and the compiler on `PATH` automatically.

Check everything with `python tdp.py doctor`. To uninstall, delete `~/.tdp3`.

## Everyday use in VSCode

| What | How |
|---|---|
| Build | **Cmd+Shift+B** |
| Build and program the board | Terminal → Run Task → **TDP3: Build + Flash** |
| See debug messages | Run Task → **TDP3: Serial monitor**, then press RESET on the board |
| Calibrate / motor test | Run Task → **TDP3: Build + Flash CALIBRATE mode** / **MOTOR TEST mode** |

Compiler errors show up in the Problems panel and are clickable.

The same commands from a terminal:

```
python tdp.py build                       # -> build/tdp3.bin
python tdp.py flash --build               # build, then copy onto the board
python tdp.py flash --build --mode calibrate
python tdp.py monitor                     # Ctrl+C to quit
```

**Flashing** copies `build/tdp3.bin` onto the board's USB drive (MBED / DAPLINK / FRDM-KL25Z). The board programs itself and remounts, and the tool checks for a `FAIL.TXT`. Plug the USB cable into the port labelled **SDA**. A macOS "disk not ejected properly" notice after flashing is normal.

## Bring-up order

1. **Motor test** (`--mode motortest`), wheels off the floor. Each wheel turns forwards, then backwards, one at a time. Fix any backwards wheel with `LEFT_INVERT` / `RIGHT_INVERT` in `src/config.h`.
2. **Calibrate** (`--mode calibrate`) on the floor. It drives straight for 2 s, pauses 5 s (yellow LED), then spins right for 2 s.
   - `CAL_MM_PER_S` = distance in mm ÷ 2
   - `CAL_DEG_PER_S` = angle in degrees ÷ 2
   - If it curved left, lower `RIGHT_TRIM_PCT`. If it curved right, lower `LEFT_TRIM_PCT`.
   - Repeat until the default rectangle path closes up.
3. **Demo path** (default mode). Edit `src/path.c`:

```c
const path_step_t g_demo_path[] = {
    FORWARD(1000),          // mm
    TURN_RIGHT(90),         // degrees, spin on the spot
    FORWARD(500),
    TURN_LEFT(45),
    WAIT(500),              // ms
    DRIVE(600, 300, 1500),  // left, right speed (permille) for 1500 ms: a curve
    BACKWARD(200),
};
```

After reset the red LED blinks for 3 s (put the robot down), then it drives (green). Blue means finished, and RESET runs it again.

Timing is open loop, so accuracy depends on calibration and battery voltage. Calibrate with the battery you will demo with.

## Code layout

| File | What it does |
|---|---|
| `src/path.c` | **the route**: edit this for the demo |
| `src/config.h` | **speeds, calibration, trims, run mode** |
| `src/pins.h` | every pin assignment (matches WIRING.md) |
| `src/main.c` | start-up, modes, LED status, crash handler (cuts motors) |
| `src/drive.c` | forward-by-mm / turn-by-degrees using the calibration |
| `src/motors.c` | L298N driver: 10 kHz PWM on ENA/ENB, ramping, stop-before-reverse |
| `src/board.c` | 1 ms SysTick timebase, `delay_ms`, RGB LED |
| `src/debug_uart.c` | `debug_printf` over the USB serial port (115200 baud) |
| `platform/` | start-up code, linker script, 48 MHz clock setup, register definitions. No need to touch these. |

Motor driver details:
- Speeds are in permille (1000 = full).
- Changes ramp at 4‰/ms (0 to full in 250 ms).
- A motor always stops for 100 ms before reversing, as the GHM-01 datasheet requires.
- At zero speed the driver brakes (EN high, both IN low).

## Troubleshooting

| Symptom | Fix |
|---|---|
| `Couldn't find the board's USB drive` | USB in the **SDA** port, data cable, or pass `--drive /Volumes/NAME` |
| Drive is called **BOOTLOADER** | You held RESET while plugging in. Replug without holding it. |
| Flashes OK but nothing happens | Press RESET. Check the monitor output. |
| Debug prints but wheels don't move | ENA/ENB jumpers still on, 12 V off, or no common GND |
| Wheel turns the wrong way | `LEFT_INVERT` / `RIGHT_INVERT` in `config.h` |
| Fast-blinking red LED | The firmware crashed (HardFault). Motors are cut. Check recent code changes. |
