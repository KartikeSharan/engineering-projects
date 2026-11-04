`timescale 1ns / 1ps  
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/04/01 
// Design Name: 
// Module Name: control_unit
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////
module control_unit (
 clk,rst,alu_end,ins,en_ram_in,en_ram_out,en_group,en_pc,reg_en,alu_in_sel,alu_func,pc_ctrl,ir_out
);
input clk,rst,alu_end,en_ram_out;	
input [15:0] ins;
output en_ram_in,en_group,en_pc,alu_in_sel;	
output [3:0]	reg_en;
output [2:0]	alu_func;
output [1:0]	pc_ctrl;
output [15:0]   ir_out ;
ir ir1 (
				.clk(clk)	,
				.rst(rst)   ,
				.ins(ins)   ,
				.en_in(en_ram_out)	,
				.ir_out(ir_out)
				);
	
	
state_transition state_transition1(
					.clk(clk) ,
					.rst(rst) ,
					.alu_end(alu_end) ,
					.rd(ir_out[11:10]) ,
					.opcode(ir_out[15:12])  ,
					.en_fetch(en_ram_in),	
					.en_group_pulse(en_group),
					.en_pc(en_pc)  ,
					.pc_ctrl(pc_ctrl) ,
					.reg_en(reg_en) ,
					.alu_in_sel(alu_in_sel)	,
					.alu_func(alu_func)
				);
endmodule







