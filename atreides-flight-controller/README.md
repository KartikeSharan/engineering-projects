# FC-Firefly: drone flight controller PCB (Atreides Industries)

Flight controller board for an early-stage drone start-up, where I worked as a voluntary electronic engineer (Jun to Nov 2025). Added now that the NDA no longer applies.

| | |
|---|---|
| Board | 6-layer, impedance-controlled, 59.6 x 66.6 mm, 2.17 mm thick |
| Parts | 25 BOM lines, 43 placements on the top side |
| Tools | KiCad (schematic, layout, ERC/DRC, Gerber and assembly outputs) |

## My role

I owned the **PCB layout**, working alongside the schematic engineer, who designed the circuit. I checked every footprint against the manufacturers' drawings, cleared ERC and DRC, kept the board's interfaces compatible with the mechanical and electronics engineers, and delivered the manufacture-ready files in `manufacturing/`.

## Folders

| Folder | Contents |
|---|---|
| `kicad/` | KiCad project, schematic and layout, plus the Bosch BMI323 (IMU) and BMM350 (magnetometer) symbol and footprint libraries |
| `manufacturing/gerbers/` | Gerbers, drill files and drill maps as sent to fabrication |
| `manufacturing/assembly/` | BOM and pick-and-place files |
