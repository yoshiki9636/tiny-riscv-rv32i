/*
 * My RISC-V RV32I CPU
 *   CPU Status Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2024 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module cpu_status (
	input clk,
	input rst_n,

	// from control
	input cpu_start,
	input quit_cmd,
	input init_cpu_start,
	// to CPU
	output reg cpu_run_state,
	output stall
	);

reg [1:0] first_edge_lat;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        first_edge_lat <= 2'b11;
    else
        first_edge_lat <= { first_edge_lat[0], 1'b0 };
end

wire first_edge = first_edge_lat[1];

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		cpu_run_state <= 1'b0;	
	else if (first_edge)
		cpu_run_state <= init_cpu_start;
	else if (quit_cmd)
		cpu_run_state <= 1'b0;	
	else if (cpu_start)
		cpu_run_state <= 1'b1;
end

assign stall = ~cpu_run_state;

endmodule
