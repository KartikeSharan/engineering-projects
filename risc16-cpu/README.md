# RISC16: a 16-bit RISC CPU in Verilog

A 16-bit load-store CPU built in **Verilog** with **Vivado** over the five-lab design strand of UESTC2001 *Application and Design of Digital Logic* (Glasgow College, UESTC, 2026), then reviewed and revised after the course.

![Full CPU, RTL schematic](report/rtl_cpu.png)

## Architecture

* **Instruction format:** `opcode[15:12] | rd[11:10] | rs[9:8] | imm[7:0]`, four 16-bit registers, 8-bit zero-extended immediate.
* **Datapath** (`data_path.v`): register file (`reg_group.v`), operand multiplexer (`alu_mux.v`), seven-operation ALU (`alu.v`: pass-B, add, subtract, AND, OR, shift left, shift right) and program counter (`pc.v`).
* **Control unit** (`control_unit.v`, `state_transition.v`): a multi-cycle FSM stepping Fetch, Decode, Execute and Write Back. An `alu_end` handshake holds Execute until the ALU result is ready.
* **Top level** (`cpu.v`) joins the two; `rom.v` holds the program.

## Who wrote what

The course supplied the module framework and, from Lab 4 on, the completed control unit. My parts of the submitted design were:

* the ALU operations (Lab 2);
* completing the AND and OR branches of the control FSM (Lab 3);
* the system testbench clock and reset, and the ROM test program (Lab 5);
* the [design report](docs/RISC16_CPU_Design_Report.pdf).

The `revised/` version (November 2026) is my own follow-up work after the course.

## Folders

| Folder | Contents |
|---|---|
| `lab2_datapath/` | Lab 2 datapath with my ALU, and the course's ALU testbench |
| `submitted/` | The final CPU as submitted with the report. The ROM program and testbench were rebuilt from the report's listings. |
| `revised/` | Corrected and extended CPU with a self-checking testbench |
| `report/`, `docs/` | Design report source and PDF |

## Review of the submitted design

Rebuilding and re-simulating the submitted CPU (it reproduces the report's results exactly) showed these problems:

1. **The test program couldn't catch ALU bugs.** Most operands were zero, AND never ran, and results were checked by eye. Of eight deliberately injected ALU bugs, it caught three.
2. **Three documented instructions hung the CPU.** MOV Rd,Rs (0001), ADD Rd,Rs (0011) and AND Rd,#imm (0110) had no Decode case, and the default left the FSM in Decode forever.
3. **The shift operations were unreachable.** No opcode selected them, so only five of the ALU's seven operations could be used.
4. **The report misread ADD.** Opcode 0010 is ADD-immediate by design. The program used an immediate of 0, so it added nothing.
5. **The testbench released reset exactly on a rising clock edge**, a race that can skip the first instruction.
6. **Synthesis inferred three latches**: `alu_func` in the FSM, and `rd_q` and `rs_q` in the register file.

## The revised design

* Every documented instruction is decoded, plus new **SHL Rd** (1011) and **SHR Rd** (1100). An undefined opcode acts as a no-op instead of hanging the FSM.
* No latches: every FSM output has a default, and the register file's read ports are a plain multiplexer.
* The ROM loads its program from `program.hex`.
* **Self-checking testbench** (`tb_cpu_selfcheck.v`): a reference model runs the same program, and each time the CPU reaches Decode the bench checks the fetched instruction and all four registers. It also fails if any opcode never ran or if the FSM hangs.
* The 15-instruction test program uses every opcode, with operands chosen so that ADD, OR and XOR give different results, and includes a subtraction that wraps below zero.

The same eight injected ALU bugs are all caught by the new bench. GitHub Actions runs it on every push.

```
cd revised && bash run_tests.sh      # needs Icarus Verilog
```

![Simulation of the submitted design](report/sim_CPU.jpeg)
