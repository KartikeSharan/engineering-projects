`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////
// state_transition: multi-cycle control FSM for the RISC16 CPU
//
// Based on the UESTC2001 course framework. Changes from the course version:
//   1. Every documented opcode is decoded, including the register forms
//      MOV Rd,Rs (0001), ADD Rd,Rs (0011) and the immediate AND Rd,#imm (0110).
//      Previously these fell into the Decode default and the CPU hung there.
//   2. New shift instructions SHL Rd (1011) and SHR Rd (1100), so the ALU's
//      shift operations are reachable from software.
//   3. Undefined opcodes are treated as a NOP (back to Fetch) instead of
//      locking the FSM in Decode.
//   4. Operand select comes from opcode[0] (1 = register Rs, 0 = immediate),
//      which matches every encoding in the instruction set table.
//   5. Every control output is assigned in every state, so synthesis infers
//      no latches (the course version left alu_func unassigned in Fetch).
//
// Instruction format: opcode[15:12] | rd[11:10] | rs[9:8] | imm[7:0]
//////////////////////////////////////////////////////////////////////////////
module state_transition(clk,rst,alu_end,rd,opcode,en_fetch,en_pc,pc_ctrl,reg_en,alu_in_sel,alu_func,en_group_pulse);

input clk,rst;
input alu_end;
input [1:0] rd;
input [3:0] opcode;
output reg en_fetch;
output reg en_pc;
output reg en_group_pulse;
output reg [1:0] pc_ctrl;
output reg [3:0] reg_en;
output reg alu_in_sel;
output reg [2:0] alu_func;

// Opcodes
localparam OP_MOVI = 4'b0000, OP_MOVR = 4'b0001,
           OP_ADDI = 4'b0010, OP_ADDR = 4'b0011,
           OP_SUBR = 4'b0101,
           OP_ANDI = 4'b0110, OP_ANDR = 4'b0111,
           OP_ORR  = 4'b1001,
           OP_JMP  = 4'b1010,
           OP_SHL  = 4'b1011, OP_SHR  = 4'b1100;

// ALU function codes (must match alu.v)
localparam F_PASSB = 3'b000, F_ADD = 3'b001, F_SUB = 3'b010, F_AND = 3'b011,
           F_OR    = 3'b100, F_SHL = 3'b101, F_SHR = 3'b110;

// States
localparam Initial       = 4'b0000,
           Fetch         = 4'b0001,
           Decode        = 4'b0010,
           Execute_Moveb = 4'b0011,
           Execute_Add   = 4'b0100,
           Execute_Sub   = 4'b0101,
           Execute_And   = 4'b0110,
           Execute_Or    = 4'b0111,
           Execute_Jump  = 4'b1000,
           Write_back    = 4'b1001,
           Execute_Shl   = 4'b1010,
           Execute_Shr   = 4'b1011;

reg en_group, en_group_reg;
reg [3:0] current_state, next_state;

always @(posedge clk or negedge rst) begin
    if (!rst) current_state <= Initial;
    else      current_state <= next_state;
end

// Next-state logic
always @(*) begin
    case (current_state)
        Initial: next_state = rst ? Fetch : Initial;
        Fetch:   next_state = Decode;
        Decode: begin
            case (opcode)
                OP_MOVI, OP_MOVR: next_state = Execute_Moveb;
                OP_ADDI, OP_ADDR: next_state = Execute_Add;
                OP_SUBR:          next_state = Execute_Sub;
                OP_ANDI, OP_ANDR: next_state = Execute_And;
                OP_ORR:           next_state = Execute_Or;
                OP_JMP:           next_state = Execute_Jump;
                OP_SHL:           next_state = Execute_Shl;
                OP_SHR:           next_state = Execute_Shr;
                default:          next_state = Fetch;      // undefined opcode: NOP
            endcase
        end
        Execute_Moveb, Execute_Add, Execute_Sub, Execute_And,
        Execute_Or, Execute_Shl, Execute_Shr:
            next_state = alu_end ? Write_back : current_state;
        Execute_Jump: next_state = Fetch;
        Write_back:   next_state = Fetch;
        default:      next_state = Initial;
    endcase
end

// Control outputs, decoded from next_state as in the course design
always @(*) begin
    // Safe defaults: nothing enabled
    en_fetch   = 1'b0;
    en_group   = 1'b0;
    en_pc      = 1'b0;
    pc_ctrl    = 2'b00;
    reg_en     = 4'b0000;
    alu_in_sel = 1'b0;
    alu_func   = F_PASSB;

    if (rst) begin
        case (next_state)
            Fetch: begin
                en_fetch = 1'b1;
                en_pc    = 1'b1;
                pc_ctrl  = 2'b01;          // PC <= PC + 1
            end
            Execute_Moveb: begin en_group = 1'b1; alu_in_sel = opcode[0]; alu_func = F_PASSB; end
            Execute_Add:   begin en_group = 1'b1; alu_in_sel = opcode[0]; alu_func = F_ADD;   end
            Execute_Sub:   begin en_group = 1'b1; alu_in_sel = opcode[0]; alu_func = F_SUB;   end
            Execute_And:   begin en_group = 1'b1; alu_in_sel = opcode[0]; alu_func = F_AND;   end
            Execute_Or:    begin en_group = 1'b1; alu_in_sel = opcode[0]; alu_func = F_OR;    end
            Execute_Shl:   begin en_group = 1'b1; alu_func = F_SHL; end
            Execute_Shr:   begin en_group = 1'b1; alu_func = F_SHR; end
            Execute_Jump: begin
                en_pc   = 1'b1;
                pc_ctrl = 2'b10;           // PC <= imm
            end
            Write_back: begin
                case (rd)
                    2'b00: reg_en = 4'b0001;
                    2'b01: reg_en = 4'b0010;
                    2'b10: reg_en = 4'b0100;
                    2'b11: reg_en = 4'b1000;
                endcase
            end
            default: ;                     // Initial, Decode: all off
        endcase
    end
end

// en_group as a one-cycle pulse on entry to an Execute state
always @(posedge clk or negedge rst) begin
    if (!rst) en_group_reg <= 1'b0;
    else      en_group_reg <= en_group;
end

always @(*) en_group_pulse = en_group & ~en_group_reg;

endmodule
