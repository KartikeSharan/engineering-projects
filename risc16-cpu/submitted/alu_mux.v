`timescale 1ns / 1ps  
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/04/01 
// Design Name: 
// Module Name: alu_mux
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
module alu_mux (
	clk,rst, 
	en_in,offset, rd_q ,rs_q,alu_in_sel,alu_a,alu_b,en_out
);
input [15:0] rd_q, rs_q;
input clk,rst,en_in,alu_in_sel;
input [7:0] offset;
output [15:0] alu_a,alu_b;
output en_out;
reg [15:0] alu_a,alu_b;
reg  en_out;

always @(posedge clk or negedge rst) 
begin
	   if(rst ==1'b0)
	   begin
			alu_a <= 16'b0000000000000000;
			alu_b <= 16'b0000000000000000;
			en_out <= 1'b0;
	   end		
	   else 
	   begin
		  if(en_in == 1'b1)
		      begin
		  	    alu_a <= rd_q;
		  	    en_out <= 1'b1;
		  		  if(alu_in_sel ==1'b0) 
		  			  alu_b <= {8'b00000000,offset[7:0]}; 
		  		  else 
		  			  alu_b <= rs_q;					
		  	   end
		  else
		  	en_out <= 1'b0;
		end
end

endmodule