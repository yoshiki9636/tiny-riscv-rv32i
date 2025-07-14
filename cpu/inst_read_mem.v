/*
 * My RISC-V RV32I CPU
 *   CPU Instruction Memory Read Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module inst_mem_read (
	input clk,
	input rst_n,
	input [31:2] pc,
	output reg [31:0] inst,

	input stall,
	input cpu_stat_imr,
	output imr_run,

	output i_read_req,
	output i_read_w,
	output i_read_hw,
	input i_read_valid,
	output [31:0] i_read_adr,
	input [31:0] i_read_data,


	);

// imr status

reg imr_stat;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		imr_stat <= 1'b0;
	else if (i_read_valid | stall)
		imr_stat <= 1'b0;
	else if (cpu_stat_imr)
		imr_stat <= 1'b1;
end

assign imr_run = imr_stat | i_read_req;

// bus control signals
assign i_read_adr = { pc, 2'd0 } ;
assign i_read_w = 1'b1;
assign i_read_hw = 1'b0;

assign i_read_req = cpu_stat_imr & ~imr_stat;

// instruction latch

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		inst <= 32'd0;
	else if (i_read_valid)
		inst <= i_read_data;
end

endmodule

