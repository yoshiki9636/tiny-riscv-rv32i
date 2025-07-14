/*
 * My RISC-V RV32I CPU
 *   CPU state machine module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module cpu_state_machine (
	input clk,
	input rst_n,
	input stall,
	input imr_run,
	input id_rfr_run,
	input dmrw_run,

	output cpu_stat_pc,
	output cpu_stat_imr,
	output cpu_stat_idrfr,
	output cpu_stat_ex,
	output cpu_stat_dmrw

	);
	


// qspi state machine

`define CPU_IDLE  3'b000
`define CPU_PC    3'b001
`define CPU_IMR   3'b010
`define CPU_IDRFR 3'b011
`define CPU_EX    3'b100
`define CPU_DMRW  3'b101

reg [2:0] cpu_state;

function [2:0] cpu_machine;
input [2:0] cpu_state;
input stall;
input imr_run;
input id_rfr_run;
input dmrw_run;


begin
	case(cpu_state)
		`CPU_IDLE: if (~stall) cpu_machine = `CPU_PC;
				  else cpu_machine = `CPU_IDLE;
		`CPU_PC: if (stall) cpu_machine = `CPU_IDLE;
				 else cpu_machine = `CPU_IMR;
		`CPU_IMR: if (stall) cpu_machine = `CPU_IDLE;
			      else if (imr_run) cpu_machine = `CPU_IMR;
				  else cpu_machine = `CPU_IDRFR;
		`CPU_IDRFR: if (stall) cpu_machine = `CPU_IDLE;
			        else if (id_rfr_run) cpu_machine = `CPU_IDRFR;
				    else cpu_machine = `CPU_ID_EX;
		`CPU_EX: if (stall) cpu_machine = `CPU_IDLE;
				 else cpu_machine = `CPU_DMRW;
		`CPU_DMRW: if (dmrw_run) cpu_machine = `CPU_DMRW;
				   else cpu_machine = `CPU_IDLE;
		default : cpu_machine = `CPU_IDLE;
	endcase
end
endfunction

wire [2:0] next_cpu_state = cpu_machine( cpu_state,
										   stall,
										   imr_run,
										   id_rfr_run,
										   dmrw_run);

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		cpu_state <= 3'b000;
	else 
		cpu_state <= next_cpu_state;
end

assign cpu_stat_pc = (cpu_state == `CPU_PC);
assign cpu_stat_imr = (cpu_state == `CPU_IMR);
assign cpu_stat_idrfr = (cpu_state == `CPU_IDRDR);
assign cpu_stat_ex = (cpu_state == `CPU_EX);
assign cpu_stat_dmrw = (cpu_state == `CPU_DMRW);

endmodule
