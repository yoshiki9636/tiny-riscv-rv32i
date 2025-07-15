/*
 * My RISC-V RV32I CPU
 *   CPU Data Access Stage Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module bus_gather (
	input clk,
	input rst_n,

	// from inst_r
	input i_read_req,
	input i_read_w,
	input i_read_hw,
	input [31:0] i_read_adr,
	
	// from data_rw
	input d_read_req,
	input d_read_w,
	input d_read_hw,
	input [31:0] d_read_adr,

	input d_write_req,
	input d_write_w,
	input d_write_hw,
	input [31:0] d_write_adr,
	input [31:0] d_write_data,

	// from uart
	input u_read_req,
	input u_read_w,
	input [31:0] u_read_adr,

	input u_write_req,
	input u_write_w,
	input [31:0] u_write_adr,
	input [31:0] u_write_data,

	// to qspi

	output read_req,
	output read_w,
	output read_hw,
	output [31:0] read_adr,
	output write_req,
	output write_w,
	output write_hw,
	output [31:0] write_adr,
	output [31:0] write_data

	);

assign read_req = i_read_req | d_read_req | u_read_req;

assign read_w = u_read_req ? u_read_w :
                i_read_req ? i_read_w : d_read_w;

assign read_hw = u_read_req ? 1'b0 :
                 i_read_req ? i_read_hw : d_read_hw;

assign read_adr = u_read_req ? u_read_adr :
                  i_read_req ? i_read_adr : d_read_adr;

assign write_req = d_write_req | u_write_req;

assign write_w = u_write_req ? u_write_w : d_write_w;

assign write_hw = ~u_write_req & d_write_hw;

assign write_adr = u_write_req ? u_write_adr : d_write_adr;

assign write_data = u_write_req ? u_write_data : d_write_data;

endmodule





