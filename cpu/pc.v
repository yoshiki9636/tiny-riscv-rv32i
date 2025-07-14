/*
 * My RISC-V RV32I CPU
 *   CPU PC Stage Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module pc_stage (
	input clk,
	input rst_n,
	input cpu_start,
	input stall,
	input cpu_stat_pc,
	input ecall_condition_ex,
	input g_interrupt,
	input g_exception,
	input jmp_condition_ex,
	input cmd_mret_ex,
	input cmd_sret_ex,
	input cmd_uret_ex,
	input [31:2] cpu_start_adr,
	input [31:2] csr_mtvec_ex,
	input [31:2] csr_mepc_ex,
	input [31:2] csr_sepc_ex,
	input [31:2] jmp_adr_ex,
	output reg [31:2] pc
	);

// resources
// PC


wire intr_ecall_exception = ecall_condition_ex | g_interrupt | g_exception ;
wire jump_cmd_cond = jmp_condition_ex | cmd_mret_ex | cmd_sret_ex | cmd_uret_ex;

wire jmp_cond = intr_ecall_exception | jump_cmd_cond;
wire [31:2] jmp_adr = intr_ecall_exception ? csr_mtvec_ex :
                      cmd_mret_ex ? csr_mepc_ex :
                      cmd_sret_ex ? csr_sepc_ex : jmp_adr_ex;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		pc <= 30'd0;
	else if (cpu_start)
		pc <= start_adr;
	else if (jmp_cond) // Causion!! keep jmp_cond to pc state
		pc <= jmp_adr;
	else (cpu_stat_pc)
		pc <= pc + 30'd1;
end


endmodule
