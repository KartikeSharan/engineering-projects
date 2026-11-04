# Line-following robot (Team Design Project 3, Team 7)

Year 3 team project (6 students, Oct 2026 to present): a robot that follows a line, detects obstacles and makes decisions. I lead the team and own the software.

* **Firmware** (`firmware/`): register-level C for the NXP FRDM-KL25Z (Arm Cortex-M0+, 48 MHz) without Mbed, built with the Arm GNU Toolchain. L298N motor driver at 10 kHz PWM with ramping and a stop before reversing, 1 ms SysTick timebase, UART debug output, a HardFault handler that cuts the motors, and calibration and motor-test modes. `tdp.py` installs the toolchain and builds, flashes and monitors the board. See `firmware/README.md` and `firmware/WIRING.md`.
* **Hardware** (`hardware/power-board/`): KiCad schematic of the power distribution board (12 V, 5 V and 3.3 V rails).
* **Project management**: the team's Gantt chart.

Status: the firmware builds but hasn't run on the robot yet; the interim demo (a pre-programmed route) is due by 20 Nov 2026.
