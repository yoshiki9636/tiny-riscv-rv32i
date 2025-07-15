/*
 * My RISC-V RV32I CPU
 *   CPU Instruction Decode Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module decoder (
	//input clk,
	//input rst_n,
	// from IF stage
	input [31:0] inst,

    // control signals
    output cmd_lui,
    output cmd_auipc,
    output [31:12] lui_auipc_imm,
    output cmd_ld,
    output [11:0] ld_alui_ofs,
    output cmd_alui,
    output cmd_alui_shamt,
    output cmd_alu,
    output cmd_alu_add,
    output cmd_alu_sub,
    output [2:0] alu_code,
    output [4:0] alui_shamt,
    output cmd_st,
    output [11:0] st_ofs,
    output cmd_jal,
    output [20:1] jal_ofs,
    output cmd_jalr,
    output [11:0] jalr_ofs,
    output cmd_br,
    output [12:1] br_ofs,
    output cmd_fence,
    output cmd_fencei,
    output [3:0] fence_succ,
    output [3:0] fence_pred,
    output cmd_sfence,
    output cmd_csr,
    output [11:0] csr_ofs,
	output [4:0] csr_uimm,
	output [2:0] csr_op2,
    output cmd_ecall,
    output cmd_ebreak,
    output cmd_uret,
    output cmd_sret,
    output cmd_mret,
    output cmd_wfi,
	output [4:0] rd_adr,
    output illegal_ops,
    output wbk_rd_reg,
	output [4:0] inst_rs1,
	output [4:0] inst_rs2
	);

// decoder
// bit slice

wire [1:0]   inst_set = inst[1:0];
wire [6:2]   inst_op1 = inst[6:2];
wire [11:7]  inst_rd  = inst[11:7];
wire [14:12] inst_op2 = inst[14:12];
assign inst_rs1 = inst[19:15];
wire [19:15] inst_uimm = inst[19:15];
assign inst_rs2 = inst[24:20];
wire [26:25] inst_op5 = inst[26:25];
wire [31:27] inst_op3 = inst[31:27];
wire [31:12] inst_imm_31_12 = inst[31:12];
wire [20:1]  inst_ofs_20_1 = { inst[31], inst[19:12], inst[20], inst[30:21] };
wire [11:0]  inst_imm_11_0 = inst[31:20];
wire [11:0]  inst_ofs_11_0_l = inst[31:20];
wire [24:20] inst_shamt = inst[24:20];
wire         inst_zero_26 = inst[26];
wire [26:25] inst_zero_26_25 = inst[26:25];
wire [23:20] inst_succ = inst[23:20];
wire [27:24] inst_pred = inst[27:24];
wire [31:28] inst_zero_31_28 = inst[31:28];
wire [19:15] inst_zero_19_15 = inst[19:15];
wire [11:7]  inst_zero_11_7  = inst[11:7];
wire [24:20] inst_op4 = inst[24:20];
wire [11:0]  inst_ofs_11_0_s = { inst[31:25], inst[11:7] };
wire [12:1]  inst_ofs_12_1 = { inst[31], inst[7], inst[30:25], inst[11:8]};

// decode opecode and zero

wire dc_notc = (inst_set == 2'b11);
wire dc_zero_26 = (inst_zero_26 == 1'b0);
wire dc_zero_26_25 = (inst_zero_26_25 == 2'd0);
wire dc_zero_31_28 = (inst_zero_31_28 == 4'd0);
wire dc_zero_19_15 = (inst_zero_19_15 == 5'd0);
wire dc_zero_11_7  = (inst_zero_11_7  == 5'd0);

wire dc_pred = (inst_pred == 4'd0);
wire dc_succ = (inst_succ == 4'd0);

//op1

function [10:0] op1_decoder;
input [4:0] inst_op1;
begin
	case(inst_op1)
		5'b01101: op1_decoder = 11'b000_0000_0001;
		5'b00101: op1_decoder = 11'b000_0000_0010;
		5'b11011: op1_decoder = 11'b000_0000_0100;
		5'b00100: op1_decoder = 11'b000_0000_1000;
		5'b11100: op1_decoder = 11'b000_0001_0000;
		5'b00000: op1_decoder = 11'b000_0010_0000;
		5'b01100: op1_decoder = 11'b000_0100_0000;
		5'b00011: op1_decoder = 11'b000_1000_0000;
		5'b11000: op1_decoder = 11'b001_0000_0000;
		5'b01000: op1_decoder = 11'b010_0000_0000;
		5'b11001: op1_decoder = 11'b100_0000_0000;
		default : op1_decoder = 11'b00_0000_0000;
	endcase
end
endfunction

wire [10:0] dc_op1 = op1_decoder( inst_op1 );

wire dc_op1_01101 = dc_op1[0];
wire dc_op1_00101 = dc_op1[1];
wire dc_op1_11011 = dc_op1[2];
wire dc_op1_00100 = dc_op1[3];
wire dc_op1_11100 = dc_op1[4];
wire dc_op1_00000 = dc_op1[5];
wire dc_op1_01100 = dc_op1[6];
wire dc_op1_00011 = dc_op1[7];
wire dc_op1_11000 = dc_op1[8];
wire dc_op1_01000 = dc_op1[9];
wire dc_op1_11001 = dc_op1[10];

// op2

function [7:0] op2_decoder;
input [2:0] inst_op2;
begin
	case(inst_op2)
		3'b000: op2_decoder = 8'b0000_0001;
		3'b001: op2_decoder = 8'b0000_0010;
		3'b010: op2_decoder = 8'b0000_0100;
		3'b011: op2_decoder = 8'b0000_1000;
		3'b100: op2_decoder = 8'b0001_0000;
		3'b101: op2_decoder = 8'b0010_0000;
		3'b110: op2_decoder = 8'b0100_0000;
		3'b111: op2_decoder = 8'b1000_0000;
		default: op2_decoder = 8'b0000_0000;
	endcase
end
endfunction

wire [7:0] dc_op2 = op2_decoder( inst_op2 );

wire dc_op2_000 = dc_op2[0];
wire dc_op2_001 = dc_op2[1];
wire dc_op2_010 = dc_op2[2];
wire dc_op2_011 = dc_op2[3];
wire dc_op2_100 = dc_op2[4];
wire dc_op2_101 = dc_op2[5];
wire dc_op2_110 = dc_op2[6];
wire dc_op2_111 = dc_op2[7];

// op3

function [3:0] op3_decoder;
input [4:0] inst_op3;
begin
	case(inst_op3)
		5'b00000: op3_decoder = 4'b0001;
		5'b01000: op3_decoder = 4'b0010;
		5'b00010: op3_decoder = 4'b0100;
		5'b00110: op3_decoder = 4'b1000;
		default : op3_decoder = 4'b0000;
	endcase
end
endfunction

wire [3:0] dc_op3 = op3_decoder( inst_op3 );

wire dc_op3_00000 = dc_op3[0];
wire dc_op3_01000 = dc_op3[1];
wire dc_op3_00010 = dc_op3[2];
wire dc_op3_00110 = dc_op3[3];

// op4
function [3:0] op4_decoder;
input [4:0] inst_op4;
begin
	case(inst_op4)
		5'b00000: op4_decoder = 4'b0001;
		5'b00001: op4_decoder = 4'b0010;
		5'b00010: op4_decoder = 4'b0100;
		5'b00101: op4_decoder = 4'b1000;
		default : op4_decoder = 4'b0000;
	endcase
end
endfunction

wire [3:0] dc_op4 = op4_decoder( inst_op4 );

wire dc_op4_00000 = dc_op4[0];
wire dc_op4_00001 = dc_op4[1];
wire dc_op4_00010 = dc_op4[2];
wire dc_op4_00101 = dc_op4[3];

// op5

wire dc_op5_01 = (inst_op5 == 2'b01);

// microcode signals

// load, auipc
assign cmd_lui = dc_op1_01101 & dc_notc;
assign cmd_auipc = dc_op1_00101 & dc_notc;
assign lui_auipc_imm = inst_imm_31_12;

assign cmd_ld = dc_op1_00000 & dc_notc;
//assign ld_ofs = inst_ofs_11_0_l;

// ALU immediate, rs2
assign cmd_alui = dc_op1_00100 & dc_notc & ~( dc_op2_001 | dc_op2_101 );
assign cmd_alui_shamt = dc_op1_00100 & dc_notc & dc_zero_26 & ( dc_op2_001 | dc_op2_101 );
assign cmd_alu = dc_op1_01100 & dc_notc & dc_zero_26_25;
assign cmd_alu_add = dc_op3_00000;
assign cmd_alu_sub = dc_op3_01000;

assign alu_code = inst_op2;

assign alui_imm = inst_imm_11_0;
assign alui_shamt = inst_shamt;

// store
assign cmd_st = dc_op1_01000 & dc_notc;
assign st_ofs = inst_ofs_11_0_s;

// jump jal jalr branch
assign cmd_jal = dc_op1_11011 & dc_notc;
assign jal_ofs = inst_ofs_20_1;

assign cmd_jalr = dc_op1_11001 & dc_op2_000 & dc_notc;
assign jalr_ofs = inst_ofs_11_0_l;

assign cmd_br = dc_op1_11000 & dc_notc;
assign br_ofs = inst_ofs_12_1;

// fence
assign cmd_fence = dc_op1_00011 & dc_op2_000 & dc_notc & dc_zero_31_28 & dc_zero_19_15 & dc_zero_11_7;
assign cmd_fencei = dc_op1_00011 & dc_op2_001 & dc_notc & dc_zero_31_28 & dc_pred & dc_succ & dc_zero_19_15 & dc_zero_11_7;
assign fence_succ = inst_succ;
assign fence_pred = inst_pred;

// sfence
assign cmd_sfence = dc_op1_11100 & dc_op2_000 & dc_notc & dc_op3_00010 & dc_op5_01;

// csr
assign cmd_csr = dc_op1_11100 & ~dc_op2_000 & dc_notc;
assign csr_ofs = inst_ofs_11_0_l;
assign csr_uimm = inst_uimm;
// need to see dc_op2_001 - dc_op2_111
assign csr_op2 = inst_op2;

// ecall
assign cmd_ec  = dc_op1_11100 &  dc_op2_000 & dc_notc & dc_zero_26_25 & dc_zero_19_15 & dc_zero_11_7;
assign cmd_ecall  = cmd_ec & dc_op3_00000 & dc_op4_00000;
assign cmd_ebreak = cmd_ec & dc_op3_00000 & dc_op4_00001;
assign cmd_uret   = cmd_ec & dc_op3_00000 & dc_op4_00010;
assign cmd_sret   = cmd_ec & dc_op3_00010 & dc_op4_00010;
assign cmd_mret   = cmd_ec & dc_op3_00110 & dc_op4_00010;
assign cmd_wfi    = cmd_ec & dc_op3_00010 & dc_op4_00101;

// nop command
assign cmd_nop = (inst == 32'h0000_0013);
// all command except nop
assign cmd_all_except_nop =
	cmd_lui | cmd_auipc | cmd_ld | cmd_alui | cmd_alui_shamt
	| cmd_alu | cmd_alu_add | cmd_alu_sub | cmd_st | cmd_jal
	| cmd_jalr | cmd_br | cmd_fence | cmd_fencei | cmd_sfence
	| cmd_csr | cmd_ec | cmd_ecall | cmd_ebreak | cmd_uret  
	| cmd_sret | cmd_mret | cmd_wfi;

assign illegal_ops = ~(cmd_nop | cmd_all_except_nop);

// destination register number
assign rd_adr = inst_rd;

// destination register write back signal

assign wbk_rd_reg = ~(cmd_st | cmd_br) & dc_notc;

// for forwarding

//assign inst_rs1_valid = cmd_alui | cmd_alui_shamt | cmd_alu | cmd_csr |
                        //cmd_sfence | cmd_ld | cmd_st | cmd_jalr | cmd_br;

//assign inst_rs2_valid = cmd_alu | cmd_st | cmd_br;

// added

assign ld_alui_ofs = inst_ofs_11_0_l;



// FF to EX stage

//always @ (posedge clk or negedge rst_n) begin   
//	if (~rst_n) begin
//        cmd_lui_ex <= 1'b0;
//        cmd_auipc_ex <= 1'b0;
//        lui_auipc_imm_ex <= 20'd0;
//        cmd_ld_ex <= 1'b0;
//        //ld_bw_ex <= 3'd0;
//        ld_alui_ofs_ex <= 12'd0;
//        cmd_alui_ex <= 1'b0;
//        cmd_alui_shamt_ex <= 1'b0;
//        cmd_alu_ex <= 1'b0;
//        cmd_alu_add_ex <= 1'b0;
//        cmd_alu_sub_ex <= 1'b0;
//        alu_code_ex <= 3'd0;
//        //alui_imm_ex <= 12'd0;
//        alui_shamt_ex <= 5'd0;
//        cmd_st_ex <= 1'b0;
//        st_ofs_ex <= 12'd0;
//        cmd_jal_ex <= 1'b0;
//        jal_ofs_ex <= 20'b0;
//        cmd_jalr_ex <= 1'b0;
//        jalr_ofs_ex <= 12'd0;
//        cmd_br_ex <= 1'b0;
//        br_ofs_ex <= 12'd0;
//        cmd_fence_ex <= 1'b0;
//        cmd_fencei_ex <= 1'b0;
//        fence_succ_ex <= 5'd0;
//        fence_pred_ex <= 5'd0;
//        cmd_sfence_ex <= 1'b0;
//        cmd_csr_ex <= 1'b0;
//        csr_ofs_ex <= 12'd0;
//		csr_uimm_ex <= 5'd0;
//		csr_op2_ex <= 3'd0;
//        cmd_ecall_ex <= 1'b0;
//        cmd_ebreak_ex <= 1'b0;
//        cmd_uret_ex <= 1'b0;
//        cmd_sret_ex <= 1'b0;
//        cmd_mret_ex <= 1'b0;
//        cmd_wfi_ex <= 1'b0;
//		illegal_ops_ex <= 1'b0;
//		rd_adr_ex <= 5'd0;
//     end
//     else
//        cmd_lui_ex <= cmd_lui & ~stall_ld;
//        cmd_auipc_ex <= cmd_auipc & ~stall_ld;
//        lui_auipc_imm_ex <= lui_auipc_imm;
//        cmd_ld_ex <= cmd_ld & ~stall_ld;
//        //ld_bw_ex <= ld_bw;
//        ld_alui_ofs_ex <= ld_ofs;
//        cmd_alui_ex <= cmd_alui & ~stall_ld;
//        cmd_alui_shamt_ex <= cmd_alui_shamt & ~stall_ld;
//        cmd_alu_ex <= cmd_alu & ~stall_ld;
//        cmd_alu_add_ex <= cmd_alu_add & ~stall_ld;
//        cmd_alu_sub_ex <= cmd_alu_sub & ~stall_ld;
//        alu_code_ex <= alu_code;
//        //alui_imm_ex <= alui_imm;
//        alui_shamt_ex <= alui_shamt;
//        cmd_st_ex <= cmd_st & ~stall_ld;
//        st_ofs_ex <= st_ofs;
//        cmd_jal_ex <= cmd_jal & ~stall_ld;
//        jal_ofs_ex <= jal_ofs;
//        cmd_jalr_ex <= cmd_jalr & ~stall_ld;
//        jalr_ofs_ex <= jalr_ofs;
//        cmd_br_ex <= cmd_br & ~stall_ld;
//        br_ofs_ex <= br_ofs;
//        cmd_fence_ex <= cmd_fence & ~stall_ld;
//        cmd_fencei_ex <= cmd_fencei & ~stall_ld;
//        fence_succ_ex <= fence_succ;
//        fence_pred_ex <= fence_pred;
//        cmd_sfence_ex <= cmd_sfence & ~stall_ld;
//        cmd_csr_ex <= cmd_csr & ~stall_ld;
//        csr_ofs_ex <= csr_ofs;
//		csr_uimm_ex <= csr_uimm;
//		csr_op2_ex <= csr_op2;
//        cmd_ecall_ex <= cmd_ecall & ~stall_ld;
//        cmd_ebreak_ex <= cmd_ebreak & ~stall_ld;
//        cmd_uret_ex <= cmd_uret & ~stall_ld;
//        cmd_sret_ex <= cmd_sret & ~stall_ld;
//        cmd_mret_ex <= cmd_mret & ~stall_ld;
//        cmd_wfi_ex <= cmd_wfi & ~stall_ld;
//		illegal_ops_ex <= illegal_ops & ~stall_ld;
//		rd_adr_ex <= rd_adr;
//    end
//end

endmodule
