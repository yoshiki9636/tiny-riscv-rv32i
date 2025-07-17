
/*
 * My RISC-V RV32I CPU
 *   CPU Top Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module cpu_top (

	input clk,
	input rst_n,
	input cpu_start,
	input quit_cmd,
	input [31:2] cpu_start_adr,
	output [31:0] pc_data,

	input interrupt_0,
	output interrupt_clear,
	output csr_mtie,
	input frc_cntr_val_leq,

	output i_read_req,
	output i_read_w,
	output i_read_hw,
	output [31:0] i_read_adr,
	output d_read_req,
	output d_read_w,
	output d_read_hw,
	input read_valid,
	output [31:0] d_read_adr,
	input [31:0] read_data,
	output d_write_req,
	output d_write_w,
	output d_write_hw,
	input write_finish,
	output [31:0] d_write_adr,
	output [31:0] d_write_data,

	output dma_io_we,
	output [15:2] dma_io_wadr,
	output [31:0] dma_io_wdata,
	output [15:2] dma_io_radr,
	output dma_io_radr_en,
	input [31:0] dma_io_rdata

	);


`define M_MODE 2'b11
`define S_MODE 2'b01
`define U_MODE 2'b00

wire stall; // output
wire [31:2] pc; // output

wire imr_run; // input
wire id_rfr_run; // input
wire dmrw_run; // input

wire cpu_stat_pc; // output
wire cpu_stat_imr; // output
wire cpu_stat_idrfr; // output
wire cpu_stat_ex; // output
wire cpu_stat_dmrw; // output

wire ecall_condition_ex; // input
wire cmd_alu_sub; // output
wire [2:0] alu_code; // output
wire [4:0] alui_shamt; // output
wire cmd_st; // output
wire [11:0] st_ofs; // output
wire cmd_jal; // output
wire [20:1] jal_ofs; // output
wire cmd_jalr; // output
wire [11:0] jalr_ofs; // output
wire cmd_br; // output
wire [12:1] br_ofs; // output
wire cmd_fence; // output
wire cmd_fencei; // output
wire [3:0] fence_succ; // output
wire [3:0] fence_pred; // output
wire cmd_sfence; // output
wire cmd_csr; // output
wire [11:0] csr_ofs; // output
wire [4:0] csr_uimm; // output
wire [2:0] csr_op2; // output
wire cmd_ecall; // output
wire cmd_ebreak; // output
wire cmd_uret; // output
wire cmd_sret; // output
wire cmd_mret; // output
wire cmd_wfi; // output
wire [4:0] rd_adr; // output
wire illegal_ops; // output
wire [4:0] inst_rs1; // input
wire [4:0] inst_rs2; // input
wire [4:0] rd_adr_wb; // input
wire wbk_rd_reg;
wire wbk_rd_reg_ma;
wire wbk_rd_reg_wb; // input
wire [31:0] wbk_data_wb; // input
wire [31:0] rs1_data; // output
wire [31:0] rs2_data; // output

//wire [31:2] pc_ex; // input
//wire wbk_rd_reg_ex; // input

wire cmd_ld_ma; // output
wire cmd_st_ma; // output
wire [4:0] rd_adr_ma; // output
wire [31:0] rd_data_ma; // output
wire [31:0] st_data_ma; // output
wire [2:0] ldst_code_ma; // output
wire [1:0] g_interrupt_priv = `M_MODE; // temp
wire [1:0] g_current_priv = `M_MODE; // temp
wire g_interrupt; // input
wire g_exception; // input
wire jmp_condition_ex; // input

wire [31:2] csr_mtvec_ex; // input
wire [31:2] csr_mepc_ex; // input
wire [31:2] csr_sepc_ex; // input
wire [31:2] jmp_adr_ex; // input
wire [31:0] inst; // output

wire cmd_lui; // output
wire cmd_auipc; // output
wire [31:12] lui_auipc_imm; // output
wire cmd_ld; // output
wire [11:0] ld_alui_ofs; // output
wire cmd_alui; // output
wire cmd_alui_shamt; // output
wire cmd_alu; // output
wire cmd_alu_add; // output
wire csr_meie; // output
wire csr_msie; // output

wire [31:2] pc_excep; // output

assign pc_data = { pc, 2'd0 };

cpu_status cpu_status (
	.clk(clk),
	.rst_n(rst_n),
	.cpu_start(cpu_start),
	.quit_cmd(quit_cmd),
	.stall(stall)
	);

cpu_state_machine cpu_state_machine (
	.clk(clk),
	.rst_n(rst_n),
	.stall(stall),
	.imr_run(imr_run),
	.id_rfr_run(id_rfr_run),
	.dmrw_run(dmrw_run),
	.cpu_stat_pc(cpu_stat_pc),
	.cpu_stat_imr(cpu_stat_imr),
	.cpu_stat_idrfr(cpu_stat_idrfr),
	.cpu_stat_ex(cpu_stat_ex),
	.cpu_stat_dmrw(cpu_stat_dmrw)
	);

pc_stage pc_stage (
	.clk(clk),
	.rst_n(rst_n),
	.cpu_start(cpu_start),
	.stall(stall),
	.cpu_stat_pc(cpu_stat_pc),
	.ecall_condition_ex(ecall_condition_ex),
	.g_interrupt(g_interrupt),
	.g_exception(g_exception),
	.jmp_condition_ex(jmp_condition_ex),
	.cmd_mret_ex(cmd_mret),
	.cmd_sret_ex(cmd_sret),
	.cmd_uret_ex(cmd_uret),
	.cpu_start_adr(cpu_start_adr),
	.csr_mtvec_ex(csr_mtvec_ex),
	.csr_mepc_ex(csr_mepc_ex),
	.csr_sepc_ex(csr_sepc_ex),
	.jmp_adr_ex(jmp_adr_ex),
	.pc(pc),
	.pc_excep(pc_excep)
	);

inst_mem_read inst_mem_read (
	.clk(clk),
	.rst_n(rst_n),
	.pc(pc),
	.inst(inst),
	.stall(stall),
	.cpu_stat_imr(cpu_stat_imr),
	.imr_run(imr_run),
	.i_read_req(i_read_req),
	.i_read_w(i_read_w),
	.i_read_hw(i_read_hw),
	.i_read_valid(read_valid),
	.i_read_adr(i_read_adr),
	.i_read_data(read_data)
	);

decoder decoder (
	.inst(inst),
	.cmd_lui(cmd_lui),
	.cmd_auipc(cmd_auipc),
	.lui_auipc_imm(lui_auipc_imm),
	.cmd_ld(cmd_ld),
	.ld_alui_ofs(ld_alui_ofs),
	.cmd_alui(cmd_alui),
	.cmd_alui_shamt(cmd_alui_shamt),
	.cmd_alu(cmd_alu),
	.cmd_alu_add(cmd_alu_add),
	.cmd_alu_sub(cmd_alu_sub),
	.alu_code(alu_code),
	.alui_shamt(alui_shamt),
	.cmd_st(cmd_st),
	.st_ofs(st_ofs),
	.cmd_jal(cmd_jal),
	.jal_ofs(jal_ofs),
	.cmd_jalr(cmd_jalr),
	.jalr_ofs(jalr_ofs),
	.cmd_br(cmd_br),
	.br_ofs(br_ofs),
	.cmd_fence(cmd_fence),
	.cmd_fencei(cmd_fencei),
	.fence_succ(fence_succ),
	.fence_pred(fence_pred),
	.cmd_sfence(cmd_sfence),
	.cmd_csr(cmd_csr),
	.csr_ofs(csr_ofs),
	.csr_uimm(csr_uimm),
	.csr_op2(csr_op2),
	.cmd_ecall(cmd_ecall),
	.cmd_ebreak(cmd_ebreak),
	.cmd_uret(cmd_uret),
	.cmd_sret(cmd_sret),
	.cmd_mret(cmd_mret),
	.cmd_wfi(cmd_wfi),
	.rd_adr(rd_adr),
	.illegal_ops(illegal_ops),
	.wbk_rd_reg(wbk_rd_reg),
	.inst_rs1(inst_rs1),
	.inst_rs2(inst_rs2)
	);

register_file register_file (
	.clk(clk),
	.rst_n(rst_n),
	.stall(stall),
	.cpu_stat_idrfr(cpu_stat_idrfr),
	.inst_rs1(inst_rs1),
	.inst_rs2(inst_rs2),
	.rd_adr_wb(rd_adr_wb),
	.wbk_rd_reg_wb(wbk_rd_reg_wb),
	.wbk_data_wb(wbk_data_wb),
	.id_rfr_run(id_rfr_run),
	.rs1_data(rs1_data),
	.rs2_data(rs2_data)
	);

execution execution (
	.clk(clk),
	.rst_n(rst_n),
	.rs1_data_ex(rs1_data),
	.rs2_data_ex(rs2_data),
	.pc_ex(pc),
	.pc_excep(pc_excep),
	.wbk_rd_reg(wbk_rd_reg),
	.cmd_lui_ex(cmd_lui),
	.cmd_auipc_ex(cmd_auipc),
	.lui_auipc_imm_ex(lui_auipc_imm),
	.cmd_ld_ex(cmd_ld),
	.ld_alui_ofs_ex(ld_alui_ofs),
	.cmd_alui_ex(cmd_alui),
	.cmd_alui_shamt_ex(cmd_alui_shamt),
	.cmd_alu_ex(cmd_alu),
	.cmd_alu_add_ex(cmd_alu_add),
	.cmd_alu_sub_ex(cmd_alu_sub),
	.alu_code_ex(alu_code),
	.alui_shamt_ex(alui_shamt),
	.cmd_st_ex(cmd_st),
	.st_ofs_ex(st_ofs),
	.cmd_jal_ex(cmd_jal),
	.jal_ofs_ex(jal_ofs),
	.cmd_jalr_ex(cmd_jalr),
	.jalr_ofs_ex(jalr_ofs),
	.cmd_br_ex(cmd_br),
	.br_ofs_ex(br_ofs),
	.cmd_fence_ex(cmd_fence),
	.cmd_fencei_ex(cmd_fencei),
	.fence_succ_ex(fence_succ),
	.fence_pred_ex(fence_pred),
	.cmd_sfence_ex(cmd_sfence),
	.cmd_csr_ex(cmd_csr),
	.csr_ofs_ex(csr_ofs),
	.csr_uimm_ex(csr_uimm),
	.csr_op2_ex(csr_op2),
	.cmd_ecall_ex(cmd_ecall),
	.cmd_ebreak_ex(cmd_ebreak),
	.cmd_uret_ex(cmd_uret),
	.cmd_sret_ex(cmd_sret),
	.cmd_mret_ex(cmd_mret),
	.cmd_wfi_ex(cmd_wfi),
	.illegal_ops_ex(illegal_ops),
	.rd_adr_ex(rd_adr),
	.cmd_ld_ma(cmd_ld_ma),
	.cmd_st_ma(cmd_st_ma),
	.wbk_rd_reg_ma(wbk_rd_reg_ma),
	.rd_adr_ma(rd_adr_ma),
	.rd_data_ma(rd_data_ma),
	.st_data_ma(st_data_ma),
	.ldst_code_ma(ldst_code_ma),
	.jmp_adr_ex(jmp_adr_ex),
	.jmp_condition_ex(jmp_condition_ex),
	.csr_mtvec_ex(csr_mtvec_ex),
	.ecall_condition_ex(ecall_condition_ex),
	.csr_mepc_ex(csr_mepc_ex),
	.csr_sepc_ex(csr_sepc_ex),
	.g_interrupt(g_interrupt),
	.g_interrupt_priv(g_interrupt_priv),
	.g_current_priv(g_current_priv),
	.g_exception(g_exception),
	.csr_meie(csr_meie),
	.csr_mtie(csr_mtie),
	.csr_msie(csr_msie),
	.cpu_stat_ex(cpu_stat_ex),
	.frc_cntr_val_leq(frc_cntr_val_leq)
	);

data_rw_mem data_rw_mem (
	.clk(clk),
	.rst_n(rst_n),
	.cmd_ld_ma(cmd_ld_ma),
	.cmd_st_ma(cmd_st_ma),
	.wbk_rd_reg_ma(wbk_rd_reg_ma),
	.rd_adr_ma(rd_adr_ma),
	.rd_data_ma(rd_data_ma),
	.st_data_ma(st_data_ma),
	.ldst_code_ma(ldst_code_ma),
	.rd_adr_wb(rd_adr_wb),
	.wbk_rd_reg_wb(wbk_rd_reg_wb),
	.wbk_data_wb(wbk_data_wb),
	.d_read_req(d_read_req),
	.d_read_w(d_read_w),
	.d_read_hw(d_read_hw),
	.read_valid(read_valid),
	.d_read_adr(d_read_adr),
	.read_data(read_data),
	.d_write_req(d_write_req),
	.d_write_w(d_write_w),
	.d_write_hw(d_write_hw),
	.write_finish(write_finish),
	.d_write_adr(d_write_adr),
	.d_write_data(d_write_data),
	.dma_io_we(dma_io_we),
	.dma_io_wadr(dma_io_wadr),
	.dma_io_wdata(dma_io_wdata),
	.dma_io_radr(dma_io_radr),
	.dma_io_radr_en(dma_io_radr_en),
	.dma_io_rdata(dma_io_rdata),
	.cpu_stat_dmrw(cpu_stat_dmrw),
	.dmrw_run(dmrw_run)
	);

interrupter interrupter (
	.clk(clk),
	.rst_n(rst_n),
	.interrupt_0(interrupt_0),
	.interrupt_clear(interrupt_clear),
	.csr_meie(csr_meie),
	.g_interrupt(g_interrupt)
	);


endmodule
