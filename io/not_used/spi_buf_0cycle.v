/*
 * My RISC-V RV32I CPU
 *   SPI 16byte buffer : 0 cycle read select
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module spi_buf_0cycle(
	input clk,
	input [1:0] ram_radr,
	output [31:0] ram_rdata,
	input [1:0] ram_wadr,
	input [31:0] ram_wdata,
	input ram_wen
	);

// 4x32 1r1w RAM

reg[31:0] ram[0:3];

always @ (posedge clk) begin
	if (ram_wen)
		ram[ram_wadr] <= ram_wdata;
end

assign ram_rdata = ram[ram_radr];

endmodule
