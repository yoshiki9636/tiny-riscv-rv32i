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
	output reg [31:2] pc,
	output [31:2] pc_excep
	);

// resources
// PC


wire intr_ecall_exception = ecall_condition_ex | g_interrupt | g_exception ;
wire jump_cmd_cond = jmp_condition_ex | cmd_mret_ex | cmd_sret_ex | cmd_uret_ex;

wire jmp_cond = intr_ecall_exception | jump_cmd_cond;
wire [31:2] jmp_adr = intr_ecall_exception ? csr_mtvec_ex :
                      cmd_mret_ex ? csr_mepc_ex :
                      cmd_sret_ex ? csr_sepc_ex : jmp_adr_ex;

//reg [31:2] pc_cntr; 
//always @ (posedge clk or negedge rst_n) begin
	//if (~rst_n)
		//pc_cntr <= 30'd0;
	//else if (cpu_start)
		//pc_cntr <= cpu_start_adr;
	//else if (jmp_cond & cpu_stat_pc) // Causion!! keep jmp_cond to pc state
		//pc_cntr <= jmp_adr;
	//else if (cpu_stat_pc)
		//pc_cntr <= pc_cntr + 30'd1;
//end

reg cpu_adr_ld;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		cpu_adr_ld <= 1'b0;
	else if (cpu_stat_pc)
		cpu_adr_ld <= 1'b0;
	else if (cpu_start)
		cpu_adr_ld <= 1'b1;
end

wire [31:2] pc_p1 = pc + 30'd1;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		pc <= 30'd0;
	else if (cpu_adr_ld & cpu_stat_pc)
		pc <= cpu_start_adr;
	else if (jmp_cond & cpu_stat_pc) // Causion!! keep jmp_cond to pc state
		pc <= jmp_adr;
	else if (cpu_stat_pc)
		pc <= pc_p1;
		//pc <= pc_cntr;
end

// pc sampler for ecall/exception
reg [31:2] pc_ecall;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		pc_ecall <= 30'd0;
	else if (ecall_condition_ex & cpu_stat_pc)
		pc_ecall <= pc;
end

assign pc_excep = (ecall_condition_ex & ~(g_interrupt | g_exception)) ? pc_ecall : pc_p1;

endmodule
