/*
 * My RISC-V RV32I CPU
 *   Control and Status Register Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module csr_array(
	input clk,
	input rst_n,

	// from ID
    input cmd_csr_ex,
    input [11:0] csr_ofs_ex,
	input [4:0] csr_uimm_ex,
	input [2:0] csr_op2_ex,
	input [31:0] rs1_sel,
	output [31:0] csr_rd_data,
	output [31:2] csr_mtvec_ex,
	input interrupts_in_pc_state,
	input g_interrupt,
	input g_interrupt_1shot,
	input illegal_ops_ex,
	input [31:0] illegal_ops_inst,
	input g_exception,
	input [1:0] g_interrupt_priv,
	input [1:0] g_current_priv,
	output [31:2] csr_mepc_ex,
	output [31:2] csr_sepc_ex,
	input cmd_mret_ex,
	input cmd_sret_ex,
	input cmd_uret_ex,
	output reg csr_rmie,
	output csr_meie,
	output csr_mtie,
	output csr_msie,
    input cmd_ecall_ex,
	input [31:2] pc_excep,
	input cpu_stat_ex,
	input cpu_stat_before_exec,
	input frc_cntr_val_leq
	);

// csr address definition

`define CSR_MSTATUS_ADR 12'h300
`define CSR_MISA_ADR 12'h301
`define CSR_MTVEC_ADR 12'h305
`define CSR_MEPC_ADR 12'h341
`define CSR_MCAUSE_ADR 12'h342
`define CSR_MTVAL_ADR 12'h343
`define CSR_MSTATUSH_ADR 12'h310
`define CSR_SEPC_ADR 12'h141
`define CSR_MIE_ADR 12'h304
`define CSR_MIP_ADR 12'h344

`define M_MODE 2'b11
`define S_MODE 2'b01
`define U_MODE 2'b00

// MISA resigister value
// MXL[31:30] : 01 : 32bit
// Extentions[25:0] : only I
`define CSR_MISA_DATA 32'h4000_0100

// op2 decode
wire immidiate = csr_op2_ex[2];
wire cmd_rw = (csr_op2_ex[1:0] == 2'b01);
wire cmd_rs = (csr_op2_ex[1:0] == 2'b10);
wire cmd_rc = (csr_op2_ex[1:0] == 2'b11);

// address decode

wire adr_mstatus = (csr_ofs_ex == `CSR_MSTATUS_ADR);
wire adr_misa = (csr_ofs_ex == `CSR_MISA_ADR);
wire adr_mtvec = (csr_ofs_ex == `CSR_MTVEC_ADR);
wire adr_mepc = (csr_ofs_ex == `CSR_MEPC_ADR);
wire adr_sepc = (csr_ofs_ex == `CSR_SEPC_ADR);
wire adr_mcause = (csr_ofs_ex == `CSR_MCAUSE_ADR);
wire adr_mtval = (csr_ofs_ex == `CSR_MTVAL_ADR);
wire adr_mstatush = (csr_ofs_ex == `CSR_MSTATUSH_ADR);
wire adr_mip = (csr_ofs_ex == `CSR_MIP_ADR);
wire adr_mie = (csr_ofs_ex == `CSR_MIE_ADR);

// read data selector
wire [31:0] csr_mstatus;
reg [31:0] csr_mstatush;
wire [31:0] csr_misa = `CSR_MISA_DATA;
reg [31:0] csr_mtvec;
reg [31:2] csr_mepc;
reg [5:0] csr_mcause;
reg [31:0] csr_mtval;
//wire [31:2] csr_sepc_i = 30'd0;
assign csr_sepc_ex = 30'd0;
wire [31:0] csr_mip;
wire [31:0] csr_mie;

wire [31:0] csr_rsel = adr_mstatus ? csr_mstatus :
                       adr_misa ? csr_misa :
                       adr_mtvec ? csr_mtvec :
                       adr_mepc ? { csr_mepc, 2'b00 } :
                       adr_sepc ? csr_sepc_ex :
                       adr_mcause ? { 26'd0, csr_mcause } :
                       adr_mtval ? csr_mtval :
                       adr_mstatush ? csr_mstatush :
                       adr_mip ? csr_mip :
                       adr_mie ? csr_mie :
                       32'd0;

// output delay latch
//reg [31:0] csr_rd_data_prev;

//always @ ( posedge clk or negedge rst_n) begin   
	//if (~rst_n) begin
		//csr_rd_data_prev <= 32'd0;
	//end
	//else if (cpu_stat_ex & cmd_csr_ex) begin
		//csr_rd_data_prev <= csr_rsel;
	//end
//end


//immidiate
//assign csr_rd_data = (cmd_rw | cmd_rs) ? csr_rd_data_prev : csr_rsel;
//assign csr_rd_data = csr_rd_data_prev;
assign csr_rd_data = csr_rsel;

// wirte data selector 
wire [31:0] wdata_rw = immidiate ? { 27'd0, csr_uimm_ex } : rs1_sel;
wire [31:0] wdata_rs = wdata_rw | csr_rsel ;
wire [31:0] wdata_rc = (~wdata_rw) & csr_rsel ;
wire [31:0] wdata_all = cmd_rw ? wdata_rw :
                        cmd_rs ? wdata_rs :
						cmd_rc ? wdata_rc : 32'd0;

// csr registers
// mstatus
wire mstatus_wr =(cpu_stat_ex)&(cmd_csr_ex)&(adr_mstatus);

reg csr_sie;
reg csr_mpie;
reg csr_spie;
reg [1:0] csr_mpp;
reg csr_spp;

// MIE[3] : Machine mode Global Interrupt enable
//wire frc_cntr_val_leq_1shot;

//wire m_interrupt = (g_interrupt_1shot | frc_cntr_val_leq_1shot) & (g_interrupt_priv == `M_MODE) & csr_rmie;
wire m_interrupt = interrupts_in_pc_state & (g_interrupt_priv == `M_MODE) & csr_rmie;
wire rmie_wr = m_interrupt | cmd_mret_ex;
wire rmie_value = m_interrupt ? 1'b0 :
                 cmd_mret_ex ? csr_mpie : csr_rmie;

always @ ( posedge clk or negedge rst_n) begin 
	if (~rst_n) begin
		csr_rmie <= 1'b0;
	end
	else if (rmie_wr) begin
		csr_rmie <= rmie_value;
	end
	else if (mstatus_wr) begin
		csr_rmie <= wdata_all[3];
	end
end

// MPIE[7] : Machine mode Previouse Interrupt Enable
wire mpie_wr = m_interrupt | cmd_mret_ex;
wire mpie_value = m_interrupt ? csr_rmie :
                  cmd_mret_ex ? 1'b1 : csr_mpie;

always @ ( posedge clk or negedge rst_n) begin 
	if (~rst_n) begin
		csr_mpie <= 1'b0;
	end
	else if (mpie_wr) begin
		csr_mpie <= mpie_value;
	end
	else if (mstatus_wr) begin
		csr_mpie <= wdata_all[7];
	end
end

// MPP[12:11] : Machine mode Previouse Privilege
wire mpp_wr = m_interrupt | cmd_mret_ex;
wire [1:0] mpp_value = m_interrupt ? g_current_priv :
                       cmd_mret_ex ? `M_MODE : // currently only M_MODE support
                       csr_mpp;

always @ ( posedge clk or negedge rst_n) begin 
	if (~rst_n) begin
		csr_mpp <= 2'b00;
	end
	else if (mpp_wr) begin
		csr_mpp <= mpp_value;
	end
	else if (mstatus_wr) begin
		csr_mpp <= wdata_all[12:11];
	end
end

// SIE[1] : Supervisor mode Global Interrupt enable : currently not used
//wire s_interrupt = (g_interrupt_1shot | frc_cntr_val_leq_1shot) & (g_interrupt_priv == `S_MODE);
wire s_interrupt = interrupts_in_pc_state & (g_interrupt_priv == `S_MODE) & csr_sie;
wire sie_wr = s_interrupt | cmd_sret_ex;
wire sie_value = s_interrupt ? 1'b0 :
                 cmd_sret_ex ? csr_spie : csr_sie;

always @ ( posedge clk or negedge rst_n) begin 
	if (~rst_n) begin
		csr_sie <= 1'b0;
	end
	else if (sie_wr) begin
		csr_sie <= sie_value;
	end
	else if (mstatus_wr) begin
		csr_sie <= wdata_all[1];
	end
end

// SPIE[5] : Supervisor mode Previouse Interrupt Enable
wire spie_wr = s_interrupt | cmd_sret_ex;
wire spie_value = s_interrupt ? csr_sie :
                  cmd_sret_ex ? 1'b1 : csr_spie;

always @ ( posedge clk or negedge rst_n) begin 
	if (~rst_n) begin
		csr_spie <= 1'b0;
	end
	else if (spie_wr) begin
		csr_spie <= spie_value;
	end
	else if (mstatus_wr) begin
		csr_spie <= wdata_all[5];
	end
end


// SPP[8] : Supervisor mode Previouse Privilege
// cueerntly fixed 0 because it dows not support S-mode
wire spp_wr = s_interrupt | cmd_sret_ex;
wire spp_value = s_interrupt ? g_current_priv :
                 cmd_sret_ex ? `U_MODE : // need to check when use the value
                 csr_spp;

always @ ( posedge clk or negedge rst_n) begin 
	if (~rst_n) begin
		csr_spp <= 1'b0;
	end
	else if (spp_wr) begin
		//csr_spp <= spp_value;
		csr_spp <= 1'b0;
	end
	else if (mstatus_wr) begin
		//csr_spp <= wdata_all[8];
		csr_spp <= 1'b0;
	end
end

assign csr_mstatus = { 19'd0, csr_mpp, 1'b0, csr_spp, 1'b0, csr_mpie,
                       1'b0, csr_spie, 1'b0, csr_rmie, 1'b0, csr_sie, 1'b0 } ;
// MPRV, MXR : is not implemented becase no U-MODE now
// SUM : is not implemented becase no S-MODE and virturalzation now
// FS,VS,XS, SD is not implemented because none of extentions are implemented
// TVM is not implemented because no virtualization implemented
// TW  is not implemented because ecurrently WFI instruction is not implemented
// TSR is not implemented because S-mode is not implemented.

// MISA : currently implimented as read-only

wire [30:0] mcause_code;

// mtvec
always @ ( posedge clk or negedge rst_n) begin   
	if (~rst_n) begin
		csr_mtvec <= 32'd0;
	end
	else if ((cpu_stat_ex)&(cmd_csr_ex)&(adr_mtvec)) begin
		csr_mtvec <= wdata_all;
	end
end

assign csr_mtvec_ex = (csr_mtvec[1:0] == 2'd0) ? csr_mtvec[31:2] :
                      (csr_mtvec[1:0] == 2'd1) ? csr_mtvec[31:2] + { 24'd0, mcause_code[5:0] } : 30'd0;


// mepc
// capture PC when ecall occured
//wire m_interrupt_in_stat_pc = m_interrupt & cpu_stat_before_exec;
//wire m_interrupt_not_stat_pc = m_interrupt & ~cpu_stat_before_exec;
//reg m_interrupt_in_stat_pc_dly;

//always @ ( posedge clk or negedge rst_n) begin   
	//if (~rst_n)
		//m_interrupt_in_stat_pc_dly <= 1'b0;
	//else if (~cpu_stat_before_exec)
		//m_interrupt_in_stat_pc_dly <= 1'b0;
	//else if (m_interrupt & cpu_stat_before_exec)
		//m_interrupt_in_stat_pc_dly <= 1'b1;
//end

//wire m_interrupt_in_stat_pc_1shot = m_interrupt_in_stat_pc_dly & ~cpu_stat_before_exec;
//wire m_interrupt_latch_timing = (m_interrupt_not_stat_pc | m_interrupt_in_stat_pc_1shot) & csr_rmie;
wire m_interrupt_latch_timing = interrupts_in_pc_state & csr_rmie;

always @ ( posedge clk or negedge rst_n) begin   
	if (~rst_n) begin
		csr_mepc <= 30'd0;
	end
	else if (cmd_ecall_ex | m_interrupt_latch_timing | g_exception) begin
		csr_mepc <= pc_excep;
	end
	else if ((cpu_stat_ex)&(cmd_csr_ex)&(adr_mepc)) begin
		csr_mepc <= wdata_all[31:2];
	end
end

assign csr_mepc_ex = csr_mepc[31:2];

// mcause
// conditions
wire interrupt_bit = g_interrupt | frc_cntr_val_leq;
// just impliment Machine mode Ecall and inteeupt
assign mcause_code = g_interrupt ? 6'd11 :
                    frc_cntr_val_leq ? 6'd7 :
                    illegal_ops_ex ? 6'd2 :
                    cmd_ecall_ex ?  6'd11 : 6'd0;
wire sel_tval = (g_interrupt | frc_cntr_val_leq) ? 32'd0 :
                illegal_ops_ex ? illegal_ops_inst : 32'd0; // need to add pc for ebreak

//wire mcause_write = cmd_ecall_ex | g_interrupt_1shot | g_exception | frc_cntr_val_leq_1shot | illegal_ops_ex;
//wire mcause_write = cmd_ecall_ex | g_exception | (interrupts_in_pc_state & csr_rmie) | illegal_ops_ex;
wire mcause_write = cmd_ecall_ex | g_exception | (interrupts_in_pc_state & csr_rmie);

always @ ( posedge clk or negedge rst_n) begin   
	if (~rst_n) begin
		csr_mcause <= 7'd0;
	end
	else if (mcause_write) begin
		csr_mcause <= { interrupt_bit, mcause_code };
	end
	else if ((cpu_stat_ex)&(cmd_csr_ex)&(adr_mcause)) begin
		csr_mcause <= { wdata_all[31], wdata_all[5:0] };
	end
end

always @ ( posedge clk or negedge rst_n) begin   
	if (~rst_n) begin
		csr_mtval <= 32'd0;
	end
	else if (mcause_write) begin
		csr_mtval <= sel_tval;
	end
	else if ((cpu_stat_ex)&(cmd_csr_ex)&(adr_mtval)) begin
		csr_mtval <= wdata_all;
	end
end

// mstatush
// [5] MBE machine level big endian -> little endian: fixed 0
// [4] SBE superviser level big endian -> little endian: fixed 0
always @ ( posedge clk or negedge rst_n) begin   
	if (~rst_n) begin
		csr_mstatush <= 32'd0;
	end
	else if ((cpu_stat_ex)&(cmd_csr_ex)&(adr_mstatush)) begin
		csr_mstatush <= { wdata_all[31:6], 2'b00, wdata_all[3:0] };
	end
end

// currently unuesd the privileges

// medelg, mideleg  is not need when the CPU dows not support S-MODE

// mip resister : currently read only register because of only M-mode is supported
// MEIP,MTIP,MSIP is set to 1 others are set to 0


// mip register
assign csr_mip = { 4'd0, g_interrupt, 3'd0, frc_cntr_val_leq, 3'd0, g_exception, 3'd0 };

// mie register
reg [2:0] csr_mie_bits;

always @ ( posedge clk or negedge rst_n) begin   
	if (~rst_n) begin
		csr_mie_bits <= 32'd0;
	end
	else if ((cpu_stat_ex)&(cmd_csr_ex)&(adr_mie)) begin
		csr_mie_bits <= { wdata_all[11], wdata_all[7], wdata_all[3] };
	end
end

assign csr_mie = { 4'd0, csr_mie_bits[2], 3'd0, csr_mie_bits[1], 3'd0, csr_mie_bits[0], 3'd0 };

assign csr_meie = csr_mie_bits[2];
assign csr_mtie = csr_mie_bits[1];
assign csr_msie = csr_mie_bits[0];

// for frc 1shot
//reg frc_cntr_val_leq_lat;

//always @ (posedge clk or negedge rst_n) begin
    //if (~rst_n)
        //frc_cntr_val_leq_lat <= 1'b0;
    //else
        //frc_cntr_val_leq_lat <= frc_cntr_val_leq;
//end

//assign frc_cntr_val_leq_1shot = frc_cntr_val_leq & ~frc_cntr_val_leq_lat;

endmodule
