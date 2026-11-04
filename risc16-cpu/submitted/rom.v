`timescale 1 ns / 1 ns
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/04/01 
// Design Name: 
// Module Name: rom
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
// Program rebuilt from the execution trace table in the CPU Design Lab Report
// (the ROM contents are blank in the course Lab Files).
module rom #(
	parameter DWIDTH = 16,
	parameter AWIDTH = 16,
	parameter DEPTH  = 16
)(
	input clk,
	input rst,
	input [AWIDTH - 1 : 0] addr,
	input ready,
	output [DWIDTH - 1 : 0] dout,
	output en_out
);
	reg [DWIDTH - 1 : 0] mem[0 : DEPTH - 1];
    reg [15:0] result=0;
	reg valid;
	reg [15:0] result_reg;

	initial begin
		mem[0] = 16'h0401;  // MOVE R1,#1
		mem[1] = 16'h0802;  // MOVE R2,#2
		mem[2] = 16'h0C08;  // MOVE R3,#8
		mem[3] = 16'h2100;  // ADD  R0,#0  (rs field not recoverable; doesn't affect the result)
		mem[4] = 16'h2200;  // ADD  R0,#0  (rs field not recoverable)
		mem[5] = 16'h0408;  // MOVE R1,#8
		mem[6] = 16'h5400;  // SUB  R1,R0
		mem[7] = 16'h9400;  // OR   R1,R0
		mem[8] = 16'h0801;  // MOVE R2,#1
		mem[9] = 16'h2400;  // ADD  R1,#0  (rs field not recoverable)
		mem[10] = 16'hA00A;  // JUMP 10
	end

	always @(posedge clk or negedge rst) begin
		if (rst == 0) begin
			result_reg <= 16'b0000_0000_0000_0000;
		end
		else begin
			result_reg <= result;
		end
	end

	always @(*) begin
		if (rst == 0) 
		begin
			result = 16'b0000_0000_0000_0000;
			valid = 0;
		end
		else 
		begin
			if (ready) 
			begin 
				result = mem[addr];	
				valid = 1;	
			end
			else 
			begin
			 	result = result_reg;
				valid = 0;
			end
		end
	end
	
assign dout = result;
assign en_out = valid;

endmodule
