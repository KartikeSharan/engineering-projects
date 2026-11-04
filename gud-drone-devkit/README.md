# Educational drone kit (Glasgow University Drones)

A solder-free, modular drone kit for the society's outreach with secondary schools: pupils assemble and programme it themselves, with JST connectors on the battery and motors and USB-C for power and programming. I proposed the kit and designed its electronics as Head of Electronics.

| | |
|---|---|
| Board | 2-layer, 70.6 x 70.6 mm |
| Main parts | ESP32-C3 module, ICM-42670-P IMU, two DRV8833 dual H-bridges (four motors), TPS63001 buck-boost, BQ24074 charger |
| Power | Buck-boost holds the 3.3 V rail across the LiPo's full range; diode-OR path from USB or battery with reverse-polarity protection |
| Status | ERC/DRC clean, first boards fabricated, not yet assembled |

## Folders

| Folder | Contents |
|---|---|
| `kicad/` | KiCad project, schematic, layout and the custom IMU and connector libraries |
| `manufacturing/` | Gerbers, drill files, BOM and pick-and-place |
| `docs/` | Schematic PDF and the chassis engineering drawing |

The chassis drawing is the mechanical team's; I coordinated PCB size and component positions, including the battery, with them.
