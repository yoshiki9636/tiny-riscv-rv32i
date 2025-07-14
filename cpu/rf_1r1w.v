/*
 * My RISC-V RV32I CPU
 *   CPU Register File RAM Modul
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module rf_1r1w(
	input clk,
	input [4:0] ram_radr,
	output [31:0] ram_rdata,
	input [4:0] ram_wadr,
	input [31:0] ram_wdata,
	input ram_wen
	);

// 4x32 1r1w RAM

reg[31:0] ram[0:31];
reg[4:0] radr;

always @ (posedge clk) begin
	if (ram_wen)
		ram[ram_wadr] <= ram_wdata;
	radr <= ram_radr;
end

assign ram_rdata = ram[radr];

endmodule
