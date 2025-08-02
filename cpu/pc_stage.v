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
	input csr_rmie,
	input ecall_condition_ex,
	input g_interrupt,
	input g_interrupt_1shot,
	input g_exception,
	input frc_cntr_val_leq,
	output interrupts_in_pc_state,
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

reg g_interrupt_latch;
wire frc_cntr_val_leq_1shot;
reg frc_cntr_val_leq_latch;

assign interrupts_in_pc_state = (g_interrupt_latch | frc_cntr_val_leq_latch) & csr_rmie & cpu_stat_pc;

wire interrupt_mskd = (g_interrupt_latch  | g_exception | frc_cntr_val_leq_latch) & csr_rmie;
wire intr_ecall_exception = ecall_condition_ex | interrupt_mskd;
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
//wire [31:2] pc_p2 = pc + 30'd2;
//wire [31:2] jmp_adr_p1 = jmp_adr + 30'd1;

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
		pc_ecall <= pc_p1;
end

assign pc_excep = (ecall_condition_ex & ~g_interrupt & ~frc_cntr_val_leq) ? pc_ecall :
                  (jmp_condition_ex) ? jmp_adr_ex : pc_p1;

                  //(jmp_cond & cpu_stat_pc) ? jmp_adr :
                  //(cpu_stat_pc) ? pc_p2 : pc_p1;
                  //(jmp_cond & cpu_stat_pc) ? jmp_adr : pc_p1;

// interrupter latch
always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		g_interrupt_latch <= 1'b0;
	else if (g_interrupt_1shot)
		g_interrupt_latch <= 1'b1;
	else if (cpu_stat_pc)
		g_interrupt_latch <= 1'b0;
end

// frc edge latch
reg frc_cntr_val_leq_lat;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		frc_cntr_val_leq_lat <= 1'b0;
	else
		frc_cntr_val_leq_lat <= frc_cntr_val_leq;
end

assign frc_cntr_val_leq_1shot = frc_cntr_val_leq & ~frc_cntr_val_leq_lat;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		frc_cntr_val_leq_latch <= 1'b0;
	else if (frc_cntr_val_leq_1shot)
		frc_cntr_val_leq_latch <= 1'b1;
	else if (cpu_stat_pc)
		frc_cntr_val_leq_latch <= 1'b0;
end

endmodule
