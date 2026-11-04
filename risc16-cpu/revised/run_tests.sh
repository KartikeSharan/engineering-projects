#!/bin/bash
# Runs the self-checking testbench with Icarus Verilog (brew install icarus-verilog).
cd "$(dirname "$0")"
iverilog -g2005 -o sim.vvp tb_cpu_selfcheck.v alu.v alu_mux.v control_unit.v cpu.v \
    data_path.v ir.v pc.v reg_group.v register.v rom.v state_transition.v && vvp -n sim.vvp
