`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////
// tb_cpu_selfcheck: self-checking testbench for the RISC16 CPU
//
// A reference model in the testbench executes the same program instruction
// by instruction. Each time the CPU reaches Decode, the testbench checks:
//   - the fetched instruction matches the model's program counter
//   - all four registers match the model (i.e. the previous instruction
//     wrote the right value to the right register and nothing else)
// It also records which opcodes ran, and fails if any opcode in the
// instruction set was never executed. Ends with PASS or FAIL.
//////////////////////////////////////////////////////////////////////////////
module tb_cpu_selfcheck;

parameter Tclk      = 10;
parameter MAX_CYCLES = 2000;
parameter PROGRAM   = "program.hex";

reg clk, rst;
wire [15:0] ins, addr;
wire en_ram_in, en_ram_out;

cpu dut (.clk(clk), .rst(rst), .addr(addr), .ins(ins),
         .en_ram_in(en_ram_in), .en_ram_out(en_ram_out));

rom #(.INIT_FILE(PROGRAM)) imem (.clk(clk), .rst(rst), .ready(en_ram_in),
         .addr(addr), .dout(ins), .en_out(en_ram_out));

initial begin clk = 1'b1; forever #(Tclk/2) clk = ~clk; end
// Release reset on a falling edge. Releasing it at exactly Tclk*12, as the
// original bench did, lands on a rising edge and races the first fetch.
initial begin rst = 1'b0; #(Tclk*12 + Tclk/2) rst = 1'b1; end

// Probes into the design
wire [3:0]  state = dut.control_unit1.state_transition1.current_state;
wire [15:0] ir    = dut.ir_out;
wire [15:0] r [0:3];
assign r[0] = dut.data_path1.reg_group1.q0;
assign r[1] = dut.data_path1.reg_group1.q1;
assign r[2] = dut.data_path1.reg_group1.q2;
assign r[3] = dut.data_path1.reg_group1.q3;
localparam S_DECODE = 4'b0010;

// Reference model
reg [15:0] prog [0:15];
reg [15:0] m [0:3];
reg [15:0] m_pc;
reg [15:0] covered;          // one bit per opcode executed
integer errors = 0, executed = 0, i, cycles = 0, last_jump_pc = -1;
reg done = 0;

localparam [15:0] REQUIRED = 16'b0001_1110_1110_1111; // opcodes 0,1,2,3,5,6,7,9,10,11,12

initial begin
    $readmemh(PROGRAM, prog);
    for (i = 0; i < 4; i = i + 1) m[i] = 16'd0;
    m_pc = 16'd0;
    covered = 16'd0;
end

task check_regs;
    integer k;
    begin
        for (k = 0; k < 4; k = k + 1)
            if (r[k] !== m[k]) begin
                errors = errors + 1;
                $display("FAIL  after instr %0d: R%0d = 0x%h, expected 0x%h", executed, k, r[k], m[k]);
            end
    end
endtask

task step_model(input [15:0] instr);
    reg [3:0] op; reg [1:0] rd, rs; reg [15:0] imm;
    begin
        op = instr[15:12]; rd = instr[11:10]; rs = instr[9:8]; imm = {8'd0, instr[7:0]};
        covered[op] = 1'b1;
        case (op)
            4'b0000: m[rd] = imm;
            4'b0001: m[rd] = m[rs];
            4'b0010: m[rd] = m[rd] + imm;
            4'b0011: m[rd] = m[rd] + m[rs];
            4'b0101: m[rd] = m[rd] - m[rs];
            4'b0110: m[rd] = m[rd] & imm;
            4'b0111: m[rd] = m[rd] & m[rs];
            4'b1001: m[rd] = m[rd] | m[rs];
            4'b1011: m[rd] = m[rd] << 1;
            4'b1100: m[rd] = m[rd] >> 1;
            default: ;                    // JMP or undefined: no register change
        endcase
        m_pc = (op == 4'b1010) ? imm : m_pc + 16'd1;
    end
endtask

// On each entry to Decode: check state, then advance the model
reg [3:0] prev_state = 4'b0000;
always @(posedge clk) begin
    if (rst && !done) begin
        cycles = cycles + 1;
        if (state == S_DECODE && prev_state != S_DECODE) begin
            check_regs;
            if (ir !== prog[m_pc]) begin
                errors = errors + 1;
                $display("FAIL  fetch: IR = 0x%h, expected prog[%0d] = 0x%h", ir, m_pc, prog[m_pc]);
            end
            // A jump to its own address is the end of the program
            if (ir[15:12] == 4'b1010 && ir[7:0] == m_pc[7:0]) begin
                if (last_jump_pc == m_pc) finish_test;
                last_jump_pc = m_pc;
            end
            if (!done) begin
                step_model(ir);
                executed = executed + 1;
            end
        end
        if (cycles > MAX_CYCLES) begin
            errors = errors + 1;
            $display("FAIL  timeout after %0d cycles (state %b): FSM hung?", cycles, state);
            finish_test;
        end
    end
    prev_state <= state;
end

task finish_test;
    begin
        done = 1;
        if ((covered & REQUIRED) !== REQUIRED) begin
            errors = errors + 1;
            $display("FAIL  opcode coverage: ran %b, need %b", covered & REQUIRED, REQUIRED);
        end
        $display("Final: R0=0x%h R1=0x%h R2=0x%h R3=0x%h after %0d instructions",
                 r[0], r[1], r[2], r[3], executed);
        if (errors == 0) $display("PASS  all register and fetch checks, every opcode executed");
        else             $display("FAIL  %0d error(s)", errors);
        $finish;
    end
endtask

endmodule
