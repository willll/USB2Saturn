# USB2Saturn Protection KiCad Skeleton

This folder contains a starter KiCad project skeleton for the protection board.

## Files

- `USB2Saturn_Protection.kicad_pro`: project settings scaffold.
- `USB2Saturn_Protection.kicad_sch`: schematic skeleton with key net labels.
- `USB2Saturn_Protection.kicad_pcb`: 2-layer PCB skeleton and board outline.
- `sym-lib-table`: local symbol library table.
- `fp-lib-table`: local footprint library table.
- `footprints.pretty/`: local footprint library folder.

## Import Existing Design Data

Use these project docs as source of truth while filling symbols/footprints:

- `docs/BOARD_BOM.csv`
- `docs/BOARD_NETLIST.csv`
- `docs/BOARD_DESIGN.md`

## Next Steps In KiCad

1. Open `USB2Saturn_Protection.kicad_pro`.
2. In schematic editor, place symbols and assign references from the BOM.
3. Apply net labels from `BOARD_NETLIST.csv`.
4. Annotate, run ERC, then update PCB from schematic.
5. Place footprints and route using rules in `BOARD_DESIGN.md`.
