`timescale 1ns / 1ps  
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2024/04/01 
// Design Name: 
// Module Name: state_transition
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
module state_transition(clk,rst,alu_end,rd,opcode,en_fetch,en_pc,pc_ctrl,reg_en,alu_in_sel,alu_func,en_group_pulse

);
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

reg en_group_reg, en_group;
reg [3:0] current_state,next_state;
parameter Initial = 4'b0000;
parameter Fetch = 4'b0001; 			
parameter Decode = 4'b0010; 		
parameter Execute_Moveb = 4'b0011;  
parameter Execute_Add = 4'b0100;    
parameter Execute_Sub = 4'b0101;    
parameter Execute_And = 4'b0110;    
parameter Execute_Or = 4'b0111;     
parameter Execute_Jump = 4'b1000;   
parameter Write_back = 4'b1001;     

always @ (posedge clk or negedge rst) begin
	if(!rst)
		current_state <= Initial;
	else 
		current_state <= next_state;
end

// Below codes defines state transition for "next_state"
always @ (*) 
begin
	case (current_state)
		Initial: 
		begin
			if(rst)
				next_state = Fetch;
			else
				next_state = Initial;
		end
		Fetch:	
		begin
			next_state = Decode;
        end
		Decode:
			begin
				case(opcode) 
                4'b0000: next_state = Execute_Moveb;
                4'b0010: next_state = Execute_Add;
                4'b0101: next_state = Execute_Sub;
                4'b0111: next_state = Execute_And;
                4'b1001: next_state = Execute_Or;
                4'b1010: next_state = Execute_Jump;
                default: next_state = current_state;
            endcase
			end
		Execute_Moveb: 
		begin
			if(alu_end)
				next_state = Write_back;
			else
				next_state = current_state;
		end
		Execute_Add: 
		begin
			if(alu_end)
				next_state = Write_back;
			else
				next_state = current_state;
		end
		Execute_Sub: 
		begin
			if(alu_end)
				next_state = Write_back;
			else
				next_state = current_state;
		end
		Execute_And: 
		begin
            if(alu_end)
				next_state = Write_back;
			else
				next_state = current_state;
		end   
		Execute_Or: 
		begin
	       if(alu_end)
                 next_state = Write_back;
           else
                 next_state = current_state;
        end  
		Execute_Jump: next_state = Fetch;
		Write_back: next_state = Fetch;
		default: next_state = current_state;
	endcase
end

// Below codes provide control signals
always @ (*) begin
	if(!rst) begin
		en_fetch = 1'b0;
		en_group = 1'b0;
		en_pc = 1'b0;
		pc_ctrl = 2'b00;
		reg_en = 4'b0000;
		alu_in_sel = 1'b0;
		alu_func = 3'b000;
	end
	else begin
		case (next_state) // Using next_state to decide control signals
			Initial: 
			begin
				en_fetch = 1'b0;
				en_group = 1'b0;
				en_pc = 1'b0;
				pc_ctrl = 2'b00;
				reg_en = 4'b0000;
				alu_in_sel = 1'b0;
				alu_func = 3'b000;
			end
			Fetch: 
			begin
				en_fetch = 1'b1;
				en_group = 1'b0;
				en_pc = 1'b1;
				pc_ctrl = 2'b01;
				reg_en = 4'b0000;
				alu_in_sel = 1'b0;
			end
			Decode: 
			begin
				en_fetch = 1'b0;
				en_group = 1'b0;
				en_pc = 1'b0;
				pc_ctrl = 2'b00;
				reg_en = 4'b0000;
				alu_in_sel = 1'b0;
				alu_func = 3'b000;
			end
			Execute_Moveb: 
			begin
				en_fetch = 1'b0;
				en_group = 1'b1;
				en_pc = 1'b0;
				pc_ctrl = 2'b00;
				reg_en = 4'b0000;
				alu_in_sel = 1'b0;
				alu_func = 3'b000;
			end
			Execute_Add: 
			begin
				en_fetch = 1'b0;
				en_group = 1'b1;
				en_pc = 1'b0;
				pc_ctrl = 2'b00;
				reg_en = 4'b0000;
				alu_in_sel = 1'b0;
				alu_func = 3'b001;
			end
			Execute_Sub: 
			begin
				en_fetch = 1'b0;
				en_group = 1'b1;
				en_pc = 1'b0;
				pc_ctrl = 2'b00;
				reg_en = 4'b0000;
				alu_in_sel = 1'b1;
				alu_func = 3'b010;
			end
			Execute_And: 
			begin
				en_fetch = 1'b0;
				en_group = 1'b1;
				en_pc = 1'b0;
				pc_ctrl = 2'b00;
				reg_en = 4'b0000;
				alu_in_sel = 1'b1;
				alu_func = 3'b011;
			end
			Execute_Or: 
			begin
				en_fetch = 1'b0;
				en_group = 1'b1; 
				en_pc = 1'b0;
				pc_ctrl = 2'b00;
				reg_en = 4'b0000;
				alu_in_sel = 1'b1;
				alu_func = 3'b100;
			end
			Execute_Jump: 
			begin
				en_fetch = 1'b0;
				en_group = 1'b0; 
				en_pc = 1'b1;
				pc_ctrl = 2'b10;
				reg_en = 4'b0000;
				alu_in_sel = 1'b0;
				alu_func = 3'b000;
			end
			Write_back:begin
			case(rd)
				2'b00: reg_en = 4'b0001;
				2'b01: reg_en = 4'b0010;
				2'b10: reg_en = 4'b0100;
				2'b11: reg_en = 4'b1000;
				default: reg_en = 4'b0000;
			endcase
				en_fetch = 1'b0;
				en_group = 1'b0;
				en_pc = 1'b0;
				pc_ctrl = 2'b00;
				alu_in_sel = 1'b0;
				alu_func = 3'b000;
			end
			default: 
			begin
				en_fetch = 1'b0;
				en_group = 1'b0;
				en_pc = 1'b0;
				pc_ctrl = 2'b00;
				reg_en = 4'b0000;
				alu_in_sel = 1'b0;
				alu_func = 3'b000;
			end
		endcase
	end
end

always @ (posedge clk or negedge rst) 
begin
	if(!rst) begin
		en_group_reg <= 1'b0;
	end
	else begin
		en_group_reg <= en_group;
	end
end

always @ (en_group_reg or en_group) 
begin
	en_group_pulse <= en_group & (~en_group_reg);
end
endmodule