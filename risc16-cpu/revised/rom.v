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
// rom: instruction memory. The program is loaded from a hex file so the
// test program can be changed without editing the RTL.
module rom #(
	parameter DWIDTH = 16,
	parameter AWIDTH = 16,
	parameter DEPTH  = 16,
	parameter INIT_FILE = "program.hex"
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
		$readmemh(INIT_FILE, mem);
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
