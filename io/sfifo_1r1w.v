/*
 * My Systolic array
 *   ram for async fifo
 *    Verilog code
 * @auther      Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight   2023 Yoshiki Kurokawa
 * @license     https://opensource.org/licenses/MIT     MIT license
 * @version     0.1
 */

module sfifo_1r1w
	#(parameter SFIFODW = 32,
	  parameter SFIFOAW = 2,
	  parameter SFIFODP = 4
    ) (
	input clk,
	input [SFIFOAW-1:0] ram_radr,
	output [SFIFODW-1:0] ram_rdata,
	input [SFIFOAW-1:0] ram_wadr,
	input [SFIFODW-1:0] ram_wdata,
	input ram_wen
	);

// 16x1024 1r1w RAM
reg[SFIFODW-1:0] ram[0:SFIFODP-1];
reg[SFIFOAW-1:0] radr;

always @ (posedge clk) begin
	if (ram_wen)
		ram[ram_wadr] <= ram_wdata;
	radr <= ram_radr;
end

assign ram_rdata = ram[radr];

endmodule
