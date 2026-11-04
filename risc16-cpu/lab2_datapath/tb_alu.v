`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/04/01 14:18:11
// Design Name: 
// Module Name: tb_alu
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description:  Test bench for alu.v
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module tb_alu();

reg clk,rst,en_in;
reg [15:0] alu_a;
reg [15:0] alu_b;
reg [2:0]  alu_func;
wire en_out;
wire [15:0] alu_out;

alu test_alu( 
                 .clk      (clk), 
                 .rst      (rst), 
	.en_in    (en_in), 
	.alu_a    (alu_a), 
	.alu_b    (alu_b), 
	.alu_func (alu_func), 
	.en_out   (en_out), 
	.alu_out  (alu_out)  
                );
parameter Tclk = 10;
initial begin
           clk=0;
           forever #(Tclk/2) clk=~clk;
end

initial begin
                           rst=0;
   #(Tclk*4)        rst=1; 
end

initial begin
                alu_func = 3'b000  ;
    #(Tclk*8)   alu_func = 3'b001  ;
	//please add rest code here
end  

initial begin
          alu_a = 16'b0000000100000001;
          alu_b = 16'b0000000000010001;
end

reg cnt1;
always @ (posedge clk) begin
	if(!rst) 
	    cnt1 <= 1'd0;
	else 
	    cnt1 <= cnt1+1;
end	

always @ (posedge clk) begin
	if(!rst)                 
	      en_in <= 1'b0;
	else if (cnt1==1'd1)
	      en_in <= 1'b1;
	else 
	      en_in <= 1'b0;      
end	

initial begin
          #(Tclk*60)  $stop;
end
	
endmodule                            
