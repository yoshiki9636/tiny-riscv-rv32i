/*
 * My RISC-V RV32I CPU
 *   CPU Data Access Stage Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module data_rw_mem (
	input clk,
	input rst_n,
	
	// from EX
    input cmd_ld_ma,
    input cmd_st_ma,
	input [4:0] rd_adr_ma,
	input [31:0] rd_data_ma,
	input [31:0] st_data_ma,
	input [2:0] ldst_code_ma,

	// to RFW
	output [4:0] rd_adr_wb,
	output wbk_rd_reg_wb,
	output [31:0] wbk_data_wb,

	// to QSPI
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

	// from/to IO
	output dma_io_we,
	output [15:2] dma_io_wadr,
	output [31:0] dma_io_wdata,
    output [15:2] dma_io_radr,
    output dma_io_radr_en,
    input [31:0] dma_io_rdata,

	// to sequencer
	input cpu_stat_dmrw,
	output dmrw_run
	);

// qspi memory
wire sel_mem =  (rd_data_ma[31:30] != 2'b11);
wire st_mem_req = cmd_st_ma & sel_mem;
wire ld_mem_req = cmd_ld_ma & sel_mem;
wire req_w =  (ldst_code_ma == 3'b010) & sel_mem;
wire req_hw = (ldst_code_ma == 3'b001) & sel_mem;

assign d_read_req = ld_mem_req;
assign d_read_w = req_w;
assign d_read_hw = req_hw;
assign d_read_adr = rd_data_ma;

assign d_write_req = st_mem_req;
assign d_write_w = req_w;
assign d_write_hw = req_hw;
assign d_write_adr = rd_data_ma;
assign d_write_data = st_data_ma;

// qspi state machine

`define DAT_IDLE 3'b000
`define DAT_READ 3'b001
`define DAT_WRTE 3'b010
`define DAT_IOR1 3'b101
`define DAT_IOR2 3'b111
`define DAT_IOWT 3'b110

reg [2:0] data_state;
//reg [3:0] data_state_dly;

function [2:0] data_machine;
input [2:0] data_state;
input st_mem_req;
input ld_mem_req;
input dma_io_radr_en;
input dma_io_we;
input read_valid;
input write_finish;

begin
	case(data_state)
		`DAT_IDLE: if (st_mem_req) data_machine = `DAT_READ;
			       else if (ld_mem_req) data_machine = `DAT_WRTE;
			       else if (dma_io_radr_en) data_machine = `DAT_IOR1;
			       else if (dma_io_we) data_machine = `DAT_IOWT;
				   else data_machine = `DAT_IDLE;
		`DAT_READ: if (read_valid) data_machine = `DAT_IDLE;
				 else data_machine = `DAT_READ;
		`DAT_WRTE: if (write_finish) data_machine = `DAT_IDLE;
				  else data_machine = `DAT_WRTE;
		`DAT_IOR1: data_machine = `DAT_IOR2;
		`DAT_IOR2: data_machine = `DAT_IDLE;
		`DAT_IOWT: data_machine = `DAT_IDLE;
		default : data_machine = `DAT_IDLE;
	endcase
end
endfunction

wire [2:0] next_data_state = data_machine( data_state,
										   st_mem_req,
										   ld_mem_req,
										   dma_io_radr_en,
										   dma_io_we,
										   read_valid,
										   write_finish);

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		data_state <= 3'b000;
	else 
		data_state <= next_data_state;
end

assign dmrw_run = (data_state != `DAT_IDLE);

// io 
assign dma_io_we = cmd_st_ma & (ldst_code_ma == 3'b010) & (rd_data_ma[31:30] == 2'b11);
assign dma_io_wadr = rd_data_ma[15:2];
assign dma_io_wdata = st_wdata;
assign dma_io_radr = rd_data_ma[15:2];
assign dma_io_radr_en = (rd_data_ma[31:30] == 2'b11) & cmd_ld_ma;

wire dma_io_ren_ma = cmd_ld_ma & (rd_data_ma[31:30] == 2'b11);

reg dma_io_ren_wb;
always @ ( posedge clk or negedge rst_n) begin   
	if (~rst_n)
		dma_io_ren_wb <= 1'b0;
	else
		dma_io_ren_wb <= dma_io_ren_ma;
end

// read data selector
assign wbk_data_wb = dma_io_ren_wb ? dma_io_rdata : write_data;
assign wbk_rd_reg_wb = dma_io_ren_wb | read_valid;
assign [4:0] rd_adr_wb = rd_adr_ma;

endmodule





