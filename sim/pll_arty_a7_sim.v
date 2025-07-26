/*
 * My RISC-V RV32I CPU
 *   PLL Dummy Module for Verilog Simulation
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2024 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module clk_wiz_0(
	output clk_out1,
	//output clk_out2,
	//output clk_out3,
	input reset,
	output locked,
	input clk_in1

	);

assign locked = 1'b1;
assign clk_out1 = clk_in1;
//assign clk_out2 = clk_in1;
//assign clk_out3 = clk_in1;

endmodule
