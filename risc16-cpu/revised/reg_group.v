`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////
// reg_group: four 16-bit registers with two read ports (rd, rs) and one
// write port (d_in, one-hot reg_en).
//
// Changed from the course version: the read ports are a plain multiplexer
// that always drives rd_q and rs_q. The course version only assigned them
// while en_in was high, so synthesis inferred two 16-bit latches to hold
// the old values. Nothing downstream needs those held values: alu_mux only
// samples rd_q and rs_q on the clock edge where en_in is high.
//////////////////////////////////////////////////////////////////////////////
module reg_group(clk, rst, en_in, reg_en, d_in, rd, rs, rd_q, rs_q, en_out);

input             clk, rst, en_in;
input      [3:0]  reg_en;
input      [15:0] d_in;
input      [1:0]  rd, rs;
output reg        en_out;
output reg [15:0] rd_q, rs_q;

wire [15:0] q0, q1, q2, q3;

register reg0 (.clk(clk), .rst(rst), .en(reg_en[0]), .d(d_in), .q(q0));
register reg1 (.clk(clk), .rst(rst), .en(reg_en[1]), .d(d_in), .q(q1));
register reg2 (.clk(clk), .rst(rst), .en(reg_en[2]), .d(d_in), .q(q2));
register reg3 (.clk(clk), .rst(rst), .en(reg_en[3]), .d(d_in), .q(q3));

always @(*) begin
    en_out = rst & en_in;
    case (rd)
        2'd0: rd_q = q0;
        2'd1: rd_q = q1;
        2'd2: rd_q = q2;
        default: rd_q = q3;
    endcase
    case (rs)
        2'd0: rs_q = q0;
        2'd1: rs_q = q1;
        2'd2: rs_q = q2;
        default: rs_q = q3;
    endcase
end

endmodule
