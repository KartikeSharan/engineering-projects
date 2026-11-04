`timescale 1ns / 1ps  
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/04/01 
// Design Name: 
// Module Name: ir
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
module ir(clk,rst,ins,en_in,ir_out);
input clk,rst;
input [15:0] ins;   
input en_in;
output reg [15:0] ir_out;

always @ (posedge clk or negedge rst) begin
	if(!rst) begin
		ir_out <= 16'b000000000000;
	end
	else begin
		if(en_in) begin
			ir_out <= ins;
		end
		else begin
			ir_out <= ir_out;
		end
	end
end
endmodule


