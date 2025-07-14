/*
 * My RISC-V RV32I CPU
 *   UART Monitor Encode and Send Character Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2021 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module uart_send_char (
    input clk,
    input rst_n,
	// from instruction/data memory
	input rdata_snd_start,
	input [31:0] rdata_snd,
	// to contorl
	output flushing_wq,
	// to uart if
	output [7:0] send_char,
	output send_en,
	input tx_fifo_full,
	input crlf_in

);


/*
reg send_mode; // 0:instruction 1:data

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        send_mode <= 1'b0;
	else if (rdata_snd_start)
		send_mode <= 1'b1;
	else if (pgm_snd_start)
        send_mode <= 1'b0;
end

wire [63:0] send_data = send_mode ? rdata_snd : inst_data_snd;
*/

wire tx_rdy = ~tx_fifo_full;

reg [4:0] send_cntr;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        send_cntr <= 5'd0;
	else if (rdata_snd_start)
        send_cntr <= 5'd9 + 5'd16;
	else if (crlf_in)
		send_cntr <= 5'd1 + 5'd16;
	else if (send_cntr[4] & tx_rdy)
         send_cntr <= send_cntr - 5'd1;
end

assign flushing_wq = (send_cntr == 5'd16) & tx_rdy;
//assign dump_cpu = send_cntr[5] & ~send_mode;

function [4:0] send_slice;
input [31:0] send_data;
input [3:0] send_cntr_low;
begin
	case(send_cntr_low)
		4'd9 : send_slice = {1'b0, send_data[31:28] };
		4'd8 : send_slice = {1'b0, send_data[27:24] };
		4'd7 : send_slice = {1'b0, send_data[23:20] };
		4'd6 : send_slice = {1'b0, send_data[19:16] };
		4'd5 : send_slice = {1'b0, send_data[15:12] };
		4'd4 : send_slice = {1'b0, send_data[11:8] };
		4'd3 : send_slice = {1'b0, send_data[7:4] };
		4'd2 : send_slice = {1'b0, send_data[3:0] };
		4'd1: send_slice = 5'h11; // CR
		4'd0 : send_slice = 5'h12; // LF
		default : send_slice = 5'h10;
	endcase
end
endfunction

wire [4:0] send_data_slice = send_slice( rdata_snd, send_cntr[3:0] );

function [7:0] send_encode;
input [4:0] send_data_slice;
begin
	case(send_data_slice)
		5'h00 : send_encode = 8'h30;
		5'h01 : send_encode = 8'h31;
		5'h02 : send_encode = 8'h32;
		5'h03 : send_encode = 8'h33;
		5'h04 : send_encode = 8'h34;
		5'h05 : send_encode = 8'h35;
		5'h06 : send_encode = 8'h36;
		5'h07 : send_encode = 8'h37;
		5'h08 : send_encode = 8'h38;
		5'h09 : send_encode = 8'h39;
		5'h0a : send_encode = 8'h61;
		5'h0b : send_encode = 8'h62;
		5'h0c : send_encode = 8'h63;
		5'h0d : send_encode = 8'h64;
		5'h0e : send_encode = 8'h65;
		5'h0f : send_encode = 8'h66;
		5'h10 : send_encode = 8'h20; // space
		5'h11 : send_encode = 8'h0d; // CR
		5'h12 : send_encode = 8'h0a; // LF
		default : send_encode = 8'h20;
	endcase
end
endfunction

assign send_char = send_encode( send_data_slice );

assign send_en = tx_rdy & send_cntr[4];

endmodule
