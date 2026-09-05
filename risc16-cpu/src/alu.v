`timescale 1 ns /1 ns
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/04/01 
// Design Name: 
// Module Name: alu
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
`define B15to0H     3'b000
`define AandBH      3'b011
`define AorBH       3'b100
`define AaddBH      3'b001
`define AsubBH      3'b010
`define leftshift   3'b101
`define rightshift  3'b110

module alu (
	clk,rst, 
	en_in, alu_a, alu_b, alu_func, en_out, alu_out
);
input [15:0] alu_a, alu_b;
input clk,rst, en_in;
input [2:0] alu_func;
output [15:0] alu_out;
output en_out;
reg [15:0] alu_out;
reg  en_out;

always @(negedge rst or posedge clk) 
begin
    if(rst ==1'b0)
	   begin
	   	alu_out <= 16'b0000000000000000;
	   	en_out  <= 1'b0;
	   end				
    else 
        begin
		if(en_in == 1'b1)
			begin
				en_out  <= 1'b1;
			    case (alu_func)
				// Please add your code here
				`B15to0H:   alu_out <= alu_b;                    // Pass through B
				`AaddBH:    alu_out <= alu_a + alu_b;           // A + B
				`AsubBH:    alu_out <= alu_a - alu_b;           // A - B
				`AandBH:    alu_out <= alu_a & alu_b;           // A & B (bitwise AND)
				`AorBH:     alu_out <= alu_a | alu_b;           // A | B (bitwise OR)
				`leftshift: alu_out <= alu_a << alu_b[3:0];     // Left shift A by B[3:0] bits
				`rightshift: alu_out <= alu_a >> alu_b[3:0];    // Right shift A by B[3:0] bits
				default:    alu_out <= 16'b0000000000000000;     // Default case
			    endcase
			end
		else 
			begin
				en_out <= 1'b0;
				alu_out <= alu_out;
			end
			
        end
end
endmodule

