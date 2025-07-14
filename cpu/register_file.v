/*
 * My RISC-V RV32I CPU
 *   CPU Register File Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module register_file (
	input clk,
	input rst_n,

	input stall,
	input cpu_stat_idrfr,
	input [4:0] inst_rs1,
	input [4:0] inst_rs2,
	input [4:0] rd_adr_wb,
	input wbk_rd_reg_wb,
	input [31:0] wbk_data_wb,

	output id_rfr_run,
	output reg [31:0] rs1_data,
	output reg [31:0] rs2_data

	);

// rfr state machine

`define RFR_IDLE 3'b000
`define RFR_ADR  3'b001
`define RFR_RS1  3'b010
`define RFR_RS2  3'b011
`define RFR_WAIT 3'b100

reg [2:0] rfr_state;

function [2:0] rfr_machine;
input [2:0] rfr_state;
input stall;
input cpu_stat_idrfr;

begin
	case(rfr_state)
		`RFR_IDLE: if (stall) rfr_machine = `RFR_IDLE;
			      else if (cpu_stat_idrfr) rfr_machine = `RFR_ADR;
				  else rfr_machine = `RFR_IDLE;
		`RFR_ADR: if (stall) rfr_machine = `RFR_WAIT;
				  else rfr_machine = `RFR_RS1;
		`RFR_RS1: if (stall) rfr_machine = `RFR_WAIT;
				  else rfr_machine = `RFR_RS2;
		`RFR_RS2: rfr_machine = `RFR_WAIT;
		`RFR_WAIT: rfr_machine = `RFR_IDLE;
		default : rfr_machine = `RFR_IDLE;
	endcase
end
endfunction

wire [2:0] next_rfr_state = rfr_machine( rfr_state,
										 stall,
										 cpu_stat_idrfr);

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		rfr_state <= 3'b000;
	else 
		rfr_state <= next_rfr_state;
end

assign id_rfr_run = (rfr_state != `RFR_IDLE)&(rfr_state != `RFR_WAIT) | (cpu_stat_idrfr & (rfr_state == `RFR_IDLE)) ;

// control singals

wire [4:8] inst_rs = (rfr_state == `RFR_ADR) ? inst_rs1 : inst_rs2;

wire rs1_zero_ex = (inst_rs1 == 5'd0);
wire rs2_zero_ex = (inst_rs2 == 5'd0);

// register file
wire [31:0] ram_data;

rf_1r1w rf_1r1w (
	.clk(clk),
	.ram_radr(inst_rs),
	.ram_rdata(ram_data),
	.ram_wadr(rd_adr_wb),
	.ram_wdata(wbk_data_wb),
	.ram_wen(wbk_rd_reg_wb)
	);


always @ (posedge clk or negedge rst_n) begin   
	if (~rst_n)
		rs1_data <= 32'd0;
	else if (rs1_zero_ex)
		rs1_data <= 32'd0;
	else if (rfr_state == `RFR_RS1)
		rs1_data <= ram_data;
end

always @ (posedge clk or negedge rst_n) begin   
	if (~rst_n)
		rs2_data <= 32'd0;
	else if (rs2_zero_ex)
		rs2_data <= 32'd0;
	else if (rfr_state == `RFR_RS2)
		rs2_data <= ram_data;
end


endmodule
