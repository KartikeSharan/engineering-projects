# Wiring: FRDM-KL25Z ↔ L298N (TDP3 Team 7)

The firmware expects exactly this. If a wire has to move, change `src/pins.h` too.

## Signals (6 wires + ground)

| FRDM pin (J1) | KL25Z pin | Firmware use | → L298N module pin | L298 IC pin |
|---|---|---|---|---|
| **D2** | PTD4  | PWM, TPM0 ch4 | **ENA** | 6 |
| **D3** | PTA12 | GPIO | **IN1** | 5 |
| **D4** | PTA4  | GPIO | **IN2** | 7 |
| **D5** | PTA5  | GPIO | **IN3** | 10 |
| **D6** | PTC8  | GPIO | **IN4** | 12 |
| **D7** | PTC9  | PWM, TPM0 ch5 | **ENB** | 11 |
| **GND** (J9) | GND | common ground | **GND** | 8 |

The order D2→D7 matches the module's header order (ENA, IN1, IN2, IN3, IN4, ENB), so a 6-way jumper bundle goes straight across.

**Leave D0/D1 (PTA1/PTA2) unconnected.** They are UART0, which carries the debug messages to the laptop over USB.

## Motors

- **Left motor** → OUT1 / OUT2 (channel A, "Output A")
- **Right motor** → OUT3 / OUT4 (channel B, "Output B")

If a wheel turns the wrong way in motor-test mode, set `LEFT_INVERT` / `RIGHT_INVERT` in `src/config.h` (or swap that motor's two wires).
Optional but recommended: a 100 nF ceramic capacitor across each motor's terminals to cut electrical noise.

## Power

| Rail | Goes to |
|---|---|
| 12 V (battery / power board) | L298 **VS** (IC pin 4, module "+12V" terminal). Nothing on the FRDM board. |
| 5 V (power board 7805) | L298 **VSS** logic supply (IC pin 9, module "+5V" terminal), **and** FRDM **VIN** (J9 pin 16) when the robot runs without USB |
| GND | Everything shares one ground: battery −, power board, L298 GND, FRDM GND |

- **Bench testing:** power the FRDM from the laptop (USB in the port labelled **SDA**). You can leave USB plugged in while the 5 V is also on VIN, because the board's USB input is diode-protected.
- **Never** put 12 V on VIN (its maximum is 9 V). **Don't** feed the power board's 3.3 V into the FRDM's 3V3 pin. That pin bypasses the board's regulator and would fight it whenever USB is connected.
- **Off-the-shelf module on the breadboard:** with a 12 V supply, leave its "5V enable" jumper **on**, so the module makes its own 5 V. In that case don't connect another 5 V to its +5V terminal. Our PCB copy has no regulator, so it takes 5 V from the power board.

## Enable jumpers: important

On the L298N module, each enable jumper joins **ENx to +5 V** (J3 ↔ J5/J7 on the schematic).

1. **Remove the ENA and ENB jumpers**, otherwise PWM speed control does nothing.
2. Connect D2/D7 to the **EN pin only**. Check with a multimeter that you're not on the **+5 V** pin of the pair. 5 V on a KL25Z pin will damage it (the KL25Z is a 3.3 V part).
3. On our PCB copy, route ENA/ENB to the MCU header and leave out the +5 V jumper pins.

## Recommended for the PCBs

- **10 kΩ pull-down resistors from ENA and ENB to GND** on the motor driver board. While the KL25Z is in reset or being programmed its pins float, and the pull-downs keep the motors off so the robot can't drive off the bench.
- The MCU distribution board breaks out D0–D7. After this, D2–D7 are taken by the motor driver and D0/D1 by USB debugging, so **there are no spare pins for the final demo's line sensors.** Please also break out the analogue header J10 (A0–A5: PTB0, PTB1, PTB2, PTB3, PTC2, PTC1) and a couple of spare digital pins.

## Logic levels

The KL25Z drives 3.3 V. The L298 reads anything ≥ 2.3 V as HIGH, so no level shifter is needed.
