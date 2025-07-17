/*
 * My RISC-V RV32I CPU
 *   RAM Module for QSPI psram Model
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module qspi_1r1w (
	input clk,
	input [16:0] ram_radr,
	output [3:0] ram_rdata,
	input [16:0] ram_wadr,
	input [3:0] ram_wdata,
	input ram_wen
	);

// 4x131072 1r1w RAM

reg[3:0] ram[0:131071];
reg[16:0] radr;

always @ (posedge clk) begin
	if (ram_wen)
		ram[ram_wadr] <= ram_wdata;
	radr <= ram_radr;
end

assign ram_rdata = ram[radr];

endmodule
