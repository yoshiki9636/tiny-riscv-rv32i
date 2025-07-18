/*
 * My RISC-V RV32I CPU
 *   QSPI inner memory module for FPGA check
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module qspi_innermem (
	input clk,
	input rst_n,

	input read_req,
	input read_w,
	input read_hw,
	output read_valid,
	input [31:0] read_adr,
	output [31:0] read_data,
	input write_req,
	input write_w,
	input write_hw,
	output write_finish,
	input [31:0] write_adr,
	input [31:0] write_data,

	input dma_io_we,
	input [15:2] dma_io_wadr,
	input [31:0] dma_io_wdata,
	input [15:2] dma_io_radr,
	input dma_io_radr_en,
	input [31:0] dma_io_rdata_in,
	output [31:0] dma_io_rdata

	);
	
wire sck;
wire [2:0] ce_n;
wire [3:0] sio_i;
wire [3:0] sio_o;

wire sio_oe1;
wire sio_oe2;

qspi_if qspi_if (
	.clk(clk),
	.rst_n(rst_n),
	.sck(sck),
	.ce_n(ce_n),
	.sio_i(sio_i),
	.sio_o(sio_o),
	.sio_oe(sio_oe1),
	.read_req(read_req),
	.read_w(read_w),
	.read_hw(read_hw),
	.read_valid(read_valid),
	.read_adr(read_adr),
	.read_data(read_data),
	.write_req(write_req),
	.write_w(write_w),
	.write_hw(write_hw),
	.write_finish(write_finish),
	.write_adr(write_adr),
	.write_data(write_data),
	.dma_io_we(dma_io_we),
	.dma_io_wadr(dma_io_wadr),
	.dma_io_wdata(dma_io_wdata),
	.dma_io_radr(dma_io_radr),
	.dma_io_radr_en(dma_io_radr_en),
	.dma_io_rdata_in(dma_io_rdata_in_4),
	.dma_io_rdata(dma_io_rdata)
	);

qspi_psram_model qspi_psram_model (
	.clk(clk),
	.rst_n(rst_n),
	.sck(sck),
	.ce_n(ce_n[0]),
	.sio_i(sio_o),
	.sio_o(sio_i),
	.sio_oe(sio_oe2)
	);

endmodule
