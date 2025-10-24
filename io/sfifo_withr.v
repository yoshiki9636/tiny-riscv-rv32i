/*
 * My RISC-V RV32I CPU
 *   sync fido with reset
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module sfifo_withr
    #(parameter SFIFODW = 8,
      parameter SFIFOAW = 3,
      parameter SFIFODP = 8
    ) (
	input clk,
	input rst_n,

    input wen,
	output wqfull,
    input [SFIFODW-1:0] wdata,
    input rnext,
	output rqempty,
    output [SFIFODW-1:0] rdata,
	input q_reset
	);

reg [SFIFOAW-1:0] wadr;
reg [SFIFOAW-1:0] radr;
wire [SFIFOAW-1:0] radr_early;

sfifo_1r1w
	#(.SFIFODW(SFIFODW),
	  .SFIFOAW(SFIFOAW),
	  .SFIFODP(SFIFODP)
	) sfifo_1r1w (
	.clk(clk),
	.ram_radr(radr_early),
	.ram_rdata(rdata),
	.ram_wadr(wadr),
	.ram_wdata(wdata),
	.ram_wen(wen)
	);

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		wadr  <= { SFIFOAW{ 1'b0 }};
	else if (q_reset)
		wadr  <= { SFIFOAW{ 1'b0 }};
	else if (wen)
		wadr  <= wadr + { { SFIFOAW-1{ 1'b0 }}, 1'b1};
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		radr  <= { SFIFOAW{ 1'b0 }};
	else if (q_reset)
		radr  <= { SFIFOAW{ 1'b0 }};
	else if (rnext)
		radr  <= radr + { { SFIFOAW-1{ 1'b0 }}, 1'b1};
end

assign radr_early = rnext ? radr + { { SFIFOAW-1{ 1'b0 }}, 1'b1} : radr;

// qfull checker
wire fwg = (wadr > radr);
wire frg = (wadr < radr);

//wire wqfull_0 = (wadr == radr);
//wire wqfull_1 = (fwg&(wadr - radr ==  SFIFODP-3))|(frg&(radr - wadr <= 3));
//wire wqfull_2 = (fwg&(wadr - radr == SFIFODP-2))|(frg&(radr - wadr <= 2));
wire wqfull_3 = (fwg&(wadr - radr == SFIFODP-1))|(frg&(radr - wadr <= 1));
//assign wqfull = wqfull_2 | wqfull_3;
assign wqfull = wqfull_3;

// qempty checker
assign rqempty = (wadr == radr_early);

endmodule
