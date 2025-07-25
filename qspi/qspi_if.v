/*
 * My RISC-V RV32I CPU
 *   QSPI interface module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module qspi_if (
	input clk,
	input rst_n,
	output reg sck,
	output reg [2:0] ce_n,
	inout [3:0] sio,
	//input [3:0] sio_i,
	//output [3:0] sio_o,
	//output sio_en,

	input [1:0] init_latency,
	input init_qspicmd,

	input read_req,
	input read_w,
	input read_hw,
	output read_valid,
	input [31:0] read_adr,
	output [31:0] read_data,
	input write_req,
	input write_w,
	input write_hw,
	output write_finish,
	input [31:0] write_adr,
	input [31:0] write_data,

	input dma_io_we,
	input [15:2] dma_io_wadr,
	input [31:0] dma_io_wdata,
	input [15:2] dma_io_radr,
	input dma_io_radr_en,
	input [31:0] dma_io_rdata_in,
	output [31:0] dma_io_rdata

	);
	
`define TERM_SCK 10'd3
`define CMD_DFREADQ 8'hEB
`define CMD_FFREADQ 8'h6B
`define CMD_DQWIRTE 8'h38
`define CMD_FQWIRTE 8'h38
`define CMD_RST_EN 8'h66
`define CMD_RESET  8'h99

// input sampler
reg word_w;
reg word_hw;
reg [25:0] word_adr;

// tristate buffer of sio
reg sio_out_enbl;
reg [3:0] sio_out;
wire [3:0] sio_in;

assign sio = sio_out_enbl ? sio_out : 4'hz;
assign sio_in = sio;

//assign sio_in = sio_i;
//assign sio_o = sio_out;
//assign sio_en = sio_out_enbl;

// input FF for meta-stable
reg [3:0] sio_in_mt0;
reg [3:0] sio_in_mt1;
reg [3:0] sio_in_sync;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n) begin
		sio_in_mt0 <= 4'h0;
		sio_in_mt1 <= 4'h0;
		sio_in_sync <= 4'h0;
	end
	else begin
		sio_in_mt0 <= sio_in;
		sio_in_mt1 <= sio_in_mt0;
		sio_in_sync <= sio_in_mt1;
	end
end

reg sck_sync;

wire rise_edge = sck & ~sck_sync;
wire fall_edge = ~sck & sck_sync;

// SCK counter
reg [9:0] sck_cntr;
reg [9:0] sck_div;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		sck_cntr <= 10'd0;
	else if (sck_cntr == sck_div)
		sck_cntr <= 10'd0;
	else
		sck_cntr <= sck_cntr + 10'd1;
end

wire half_sck = (sck_cntr == sck_div);

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		sck <= 1'd1;
	else if (half_sck)
		sck <= ~sck;
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		sck_sync <= 1'd1;
	else
		sck_sync <= sck;
end

// state machine control signals

wire cmd_end;
wire adr_end;
wire rst_end;
wire cmd_freadq;
wire cmd_qwrite;
wire cmd_rst_en = 1'b0; // temp
wire read_wait_end;
wire read_data_end;
wire write_data_end;

// qspi state machine

`define QS_IDLE  4'b0000
`define QS_CMD   4'b0001
`define QS_ADR   4'b0010
`define QS_WTDAT 4'b0011
`define QS_RDWIT 4'b0101
`define QS_RDDAT 4'b0111
`define QS_RSTEN 4'b1000
`define QS_RESET 4'b1001

reg [3:0] qspi_state;
//reg [3:0] qspi_state_dly;

function [3:0] qspi_machine;
input [3:0] qspi_state;
input cmd_end;
input adr_end;
input rst_end;
input cmd_freadq;
input cmd_qwrite;
input cmd_rst_en;
input read_wait_end;
input read_data_end;
input write_data_end;


begin
	case(qspi_state)
		`QS_IDLE: if (cmd_freadq | cmd_qwrite | cmd_rst_en) qspi_machine = `QS_CMD;
				  else qspi_machine = `QS_IDLE;
		`QS_CMD: if (cmd_end & (cmd_freadq | cmd_qwrite)) qspi_machine = `QS_ADR;
				 else if (cmd_end & cmd_rst_en) qspi_machine = `QS_RSTEN;
				 else qspi_machine = `QS_CMD;
		`QS_ADR: if (adr_end & cmd_freadq) qspi_machine = `QS_RDWIT;
			     else if (adr_end & cmd_qwrite) qspi_machine = `QS_WTDAT;
				 else qspi_machine = `QS_ADR;
		`QS_WTDAT: if (write_data_end) qspi_machine = `QS_IDLE;
				   else qspi_machine = `QS_WTDAT;
		`QS_RDWIT: if (read_wait_end) qspi_machine = `QS_RDDAT;
				   else qspi_machine = `QS_RDWIT;
		`QS_RDDAT: if (read_data_end) qspi_machine = `QS_IDLE;
				   else qspi_machine = `QS_RDDAT;
		`QS_RSTEN: if (rst_end) qspi_machine = `QS_RESET;
				   else qspi_machine = `QS_RSTEN;
		`QS_RESET: if (rst_end) qspi_machine = `QS_IDLE;
				   else qspi_machine = `QS_RESET;
		default : qspi_machine = `QS_IDLE;
	endcase
end
endfunction

wire [3:0] next_qspi_state = qspi_machine( qspi_state,
										   cmd_end,
										   adr_end,
										   rst_end,
										   cmd_freadq,
										   cmd_qwrite,
										   cmd_rst_en,
										   read_wait_end,
										   read_data_end,
										   write_data_end);

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n) begin
		qspi_state <= 4'b0000;
		//qspi_state_dly <= 4'b0000;
	end
	else if (fall_edge) begin
		qspi_state <= next_qspi_state;
		//qspi_state_dly <= qspi_state;
	end
end

wire state_cmd = (qspi_state == `QS_CMD);
wire next_state_cmd = (next_qspi_state == `QS_CMD);
wire state_adr = (qspi_state == `QS_ADR);
wire next_state_adr = (next_qspi_state == `QS_ADR);
wire state_write = (qspi_state == `QS_WTDAT);
wire next_state_write = (next_qspi_state == `QS_WTDAT);
wire state_rdwt = (qspi_state == `QS_RDWIT);
wire next_state_rdwt = (next_qspi_state == `QS_RDWIT);
wire state_read = (qspi_state == `QS_RDDAT);
wire next_state_read = (next_qspi_state == `QS_RDDAT);
wire state_rsten = (qspi_state == `QS_RSTEN);
wire next_state_rsten = (next_qspi_state == `QS_RSTEN);
wire state_rst = (qspi_state == `QS_RESET);
wire next_state_rst = (next_qspi_state == `QS_RESET);

wire ce_n_pre = (qspi_state == `QS_IDLE);
wire ce_0_dec = (word_adr[25:24] == 2'd0);
wire ce_1_dec = (word_adr[25:24] == 2'd1);
wire ce_2_dec = (word_adr[25:24] == 2'd2);
wire ce_0_en = ~ce_0_dec | ce_n_pre;
wire ce_1_en = ~ce_1_dec | ce_n_pre;
wire ce_2_en = ~ce_2_dec | ce_n_pre;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		ce_n <= 3'b111;
	else
		ce_n <= { ce_2_en, ce_1_en, ce_0_en };
end

// command slicer
wire [7:0] cmd_byte;
wire cmd_bit;
reg [2:0] cmd_ofs;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		cmd_ofs <= 3'd0;
	else if (~state_cmd & next_state_cmd & fall_edge)
		cmd_ofs <= 3'd7;
	else if ( state_cmd & fall_edge)
		cmd_ofs <= cmd_ofs - 3'd1;
end

wire cmd_rsten = 1'b0; // temp

reg [7:0] rdcmd0;
reg [7:0] rdcmd1;
reg [7:0] wrcmd0;
reg [7:0] wrcmd1;

reg [5:0] rdwrch;

wire [7:0] cmd_read_ce_n0 =  (rdwrch[0]) ? rdcmd1 : rdcmd0;
wire [7:0] cmd_write_ce_n0 = (rdwrch[1]) ? wrcmd1 : wrcmd0;
wire [7:0] cmd_read_ce_n1 =  (rdwrch[2]) ? rdcmd1 : rdcmd0;
wire [7:0] cmd_write_ce_n1 = (rdwrch[3]) ? wrcmd1 : wrcmd0;
wire [7:0] cmd_read_ce_n2 =  (rdwrch[4]) ? rdcmd1 : rdcmd0;
wire [7:0] cmd_write_ce_n2 = (rdwrch[5]) ? wrcmd1 : wrcmd0;

wire [7:0] cmd_read_sel =  (~ce_2_en) ? cmd_read_ce_n2 :
                           (~ce_1_en) ? cmd_read_ce_n1 : cmd_read_ce_n0;
wire [7:0] cmd_write_sel =  (~ce_2_en) ? cmd_write_ce_n2 :
                            (~ce_1_en) ? cmd_write_ce_n1 : cmd_write_ce_n0;

assign cmd_byte = (cmd_freadq) ? cmd_read_sel :
                  (cmd_qwrite) ? cmd_write_sel :
                  (cmd_rsten) ? `CMD_RST_EN : `CMD_RESET;

assign cmd_bit =  cmd_byte[cmd_ofs];

assign cmd_end = state_cmd & (cmd_ofs == 3'd0) & fall_edge;

// adr slicer
wire [23:0] adr_rw;
wire [3:0] adr_slice;
reg [2:0] adr_ofs;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		adr_ofs <= 3'd0;
	else if (~state_adr & next_state_adr & fall_edge)
		adr_ofs <= 3'd5;
	else if ( state_adr & fall_edge)
		adr_ofs <= adr_ofs - 3'd1;
end

assign adr_rw = word_adr[23:0];

assign adr_slice = (adr_ofs == 3'd5) ? adr_rw[23:20] :
                   (adr_ofs == 3'd4) ? adr_rw[19:16] :
                   (adr_ofs == 3'd3) ? adr_rw[15:12] :
                   (adr_ofs == 3'd2) ? adr_rw[11:8] :
                   (adr_ofs == 3'd1) ? adr_rw[7:4] : adr_rw[3:0];

assign adr_end = state_adr & (adr_ofs == 3'b0) & fall_edge;

// write data slicer
reg [31:0] write_data_lat;

wire [31:0] wdata = write_data_lat;
wire [3:0] wdata_slice;
reg [2:0] wdata_ofs;

wire [2:0] write_length = word_w ? 3'd7 :
                          word_hw ? 3'd3 : 3'd1;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		wdata_ofs <= 3'd0;
	else if (~state_write & next_state_write & fall_edge)
		wdata_ofs <= write_length;
	else if ( state_write & fall_edge)
		wdata_ofs <= wdata_ofs - 3'd1;
end

wire [31:0] ext_wdata = word_w ? wdata :
                        word_hw ? { wdata[15:0], 16'd0 } : { wdata[7:0], 24'd0 } ;


assign wdata_slice = (wdata_ofs == 3'd1) ? ext_wdata[31:28] :
                     (wdata_ofs == 3'd0) ? ext_wdata[27:24] :
                     (wdata_ofs == 3'd3) ? ext_wdata[23:20] :
                     (wdata_ofs == 3'd2) ? ext_wdata[19:16] :
                     (wdata_ofs == 3'd5) ? ext_wdata[15:12] :
                     (wdata_ofs == 3'd4) ? ext_wdata[11:8] :
                     (wdata_ofs == 3'd7) ? ext_wdata[7:4] : ext_wdata[3:0];

assign write_data_end = state_write & (wdata_ofs == 3'b0) & fall_edge;

// final selector
wire [3:0] sio_out_pre = (state_cmd) ? { 3'b000, cmd_bit } :
                         (state_adr) ? adr_slice :
                         (state_write) ? wdata_slice : 4'd0;

wire sio_out_enbl_pre = state_cmd | state_adr | state_write;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n) begin
		sio_out <= 4'd0;
		sio_out_enbl <= 1'b0;
	end
	else begin
		sio_out <= sio_out_pre;
		sio_out_enbl <= sio_out_enbl_pre;
	end
end

// read data sampler
reg [31:0] word_data;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		word_data <= 32'd0;
	else if (~state_read & next_state_read & fall_edge)
		//word_data <= { 28'd0, sio_in_sync } ;
		word_data <= 32'd0;
	else if ( state_read & rise_edge)
		word_data <= { word_data[27:0], sio_in_sync } ;
end

assign read_data = word_w ? { word_data[7:0], word_data[15:8], word_data[23:16], word_data[31:24] } :
                   word_hw ? { 16'd0, word_data[7:0], word_data[15:8] } : { 24'd0, word_data[7:0] };

// reset counter
reg [3:0] rst_cntr;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 rst_cntr <= 4'd0;
	else if ((~state_rsten & next_state_rsten) | (~state_rst & next_state_rst))
		 rst_cntr <= 4'd8;
	else if (rst_cntr == 4'd0)
		 rst_cntr <= 4'd0;
	else if (fall_edge)
		 rst_cntr <= rst_cntr - 4'd1;
end

assign rst_end = (state_rsten | state_rst) & (rst_cntr == 4'd0) & fall_edge;

// read latency register
`define SYS_QSPI_LATENCY0 14'h3D00
`define SYS_QSPI_LATENCY1 14'h3D01
`define SYS_QSPI_LATENCY2 14'h3D02
`define SYS_QSPI_SCKDIV   14'h3D03
`define SYS_QSPI_RDCMD0   14'h3D04
`define SYS_QSPI_RDCMD1   14'h3D05
`define SYS_QSPI_WRCMD0   14'h3D06
`define SYS_QSPI_WRCMD1   14'h3D07
`define SYS_QSPI_RDWTCH   14'h3D08

wire we_qspi_latency0 = dma_io_we      & (dma_io_wadr == `SYS_QSPI_LATENCY0);
wire re_qspi_latency0 = dma_io_radr_en & (dma_io_radr == `SYS_QSPI_LATENCY0);
wire we_qspi_latency1 = dma_io_we      & (dma_io_wadr == `SYS_QSPI_LATENCY1);
wire re_qspi_latency1 = dma_io_radr_en & (dma_io_radr == `SYS_QSPI_LATENCY1);
wire we_qspi_latency2 = dma_io_we      & (dma_io_wadr == `SYS_QSPI_LATENCY2);
wire re_qspi_latency2 = dma_io_radr_en & (dma_io_radr == `SYS_QSPI_LATENCY2);
wire we_qspi_sckdiv = dma_io_we      & (dma_io_wadr == `SYS_QSPI_SCKDIV);
wire re_qspi_sckdiv = dma_io_radr_en & (dma_io_radr == `SYS_QSPI_SCKDIV);

wire we_qspi_rdcmd0 = dma_io_we      & (dma_io_wadr == `SYS_QSPI_RDCMD0);
wire re_qspi_rdcmd0 = dma_io_radr_en & (dma_io_radr == `SYS_QSPI_RDCMD0);
wire we_qspi_rdcmd1 = dma_io_we      & (dma_io_wadr == `SYS_QSPI_RDCMD1);
wire re_qspi_rdcmd1 = dma_io_radr_en & (dma_io_radr == `SYS_QSPI_RDCMD1);
wire we_qspi_wrcmd0 = dma_io_we      & (dma_io_wadr == `SYS_QSPI_WRCMD0);
wire re_qspi_wrcmd0 = dma_io_radr_en & (dma_io_radr == `SYS_QSPI_WRCMD0);
wire we_qspi_wrcmd1 = dma_io_we      & (dma_io_wadr == `SYS_QSPI_WRCMD1);
wire re_qspi_wrcmd1 = dma_io_radr_en & (dma_io_radr == `SYS_QSPI_WRCMD1);
wire we_qspi_rdwrch = dma_io_we      & (dma_io_wadr == `SYS_QSPI_RDWTCH);
wire re_qspi_rdwrch = dma_io_radr_en & (dma_io_radr == `SYS_QSPI_RDWTCH);


reg [3:0] read_latency_0;
reg [3:0] read_latency_1;
reg [3:0] read_latency_2;

// causion!! need to change if default memory does not work
wire [3:0] init_latency_value_0 = (init_latency == 2'd0) ? 4'd7 :
                                  (init_latency == 2'd1) ? 4'd8 :
                                  (init_latency == 2'd2) ? 4'd9 : 4'd6;

wire [3:0] init_latency_value_1 = (init_latency == 2'd0) ? 4'd7 :
                                  (init_latency == 2'd1) ? 4'd8 :
                                  (init_latency == 2'd2) ? 4'd9 : 4'd6;

wire [3:0] init_latency_value_2 = (init_latency == 2'd0) ? 4'd7 :
                                  (init_latency == 2'd1) ? 4'd8 :
                                  (init_latency == 2'd2) ? 4'd9 : 4'd6;

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
		 read_latency_0 <= 4'd0;
	else if (first_edge)
		 read_latency_0 <= init_latency_value_0;
	else if (we_qspi_latency0)
		 read_latency_0 <= dma_io_wdata[3:0];
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 read_latency_1 <= 4'd0;
	else if (first_edge)
		 read_latency_1 <= init_latency_value_1;
	else if (we_qspi_latency1)
		 read_latency_1 <= dma_io_wdata[3:0];
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 read_latency_2 <= 4'd0;
	else if (first_edge)
		 read_latency_2 <= init_latency_value_2;
	else if (we_qspi_latency2)
		 read_latency_2 <= dma_io_wdata[3:0];
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 sck_div <= `TERM_SCK;
	else if (we_qspi_sckdiv)
		 sck_div <= dma_io_wdata[9:0];
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 rdcmd0 <= `CMD_DFREADQ;
	else if (we_qspi_rdcmd0)
		 rdcmd0 <= dma_io_wdata[7:0];
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 rdcmd1 <= `CMD_FFREADQ;
	else if (we_qspi_rdcmd1)
		 rdcmd1 <= dma_io_wdata[7:0];
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 wrcmd0 <= `CMD_DQWIRTE;
	else if (we_qspi_wrcmd0)
		 wrcmd0 <= dma_io_wdata[7:0];
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 wrcmd1 <= `CMD_FQWIRTE;
	else if (we_qspi_wrcmd1)
		 wrcmd1 <= dma_io_wdata[7:0];
end

wire [5:0] init_rdwrch = init_qspicmd ? 6'b000011 : 6'b000000;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 rdwrch <= 6'd0;
	else if (first_edge)
		 rdwrch <= init_rdwrch;
	else if (we_qspi_rdwrch)
		 rdwrch <= dma_io_wdata[5:0];
end

reg [8:0] re_qspi_latency_dly;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 re_qspi_latency_dly <= 9'd0;
	else
		 re_qspi_latency_dly <= { re_qspi_rdwrch, re_qspi_wrcmd1, re_qspi_wrcmd0, re_qspi_rdcmd1, re_qspi_rdcmd0,
                                  re_qspi_sckdiv, re_qspi_latency2, re_qspi_latency1, re_qspi_latency0 };
end

assign dma_io_rdata = (re_qspi_latency_dly[0] == 1'b1) ? { 28'd0, read_latency_0 } :
                      (re_qspi_latency_dly[1] == 1'b1) ? { 28'd0, read_latency_1 } :
                      (re_qspi_latency_dly[2] == 1'b1) ? { 28'd0, read_latency_2 } :
                      (re_qspi_latency_dly[3] == 1'b1) ? { 22'd0, sck_div } :
                      (re_qspi_latency_dly[4] == 1'b1) ? { 24'd0, rdcmd0 } :
                      (re_qspi_latency_dly[5] == 1'b1) ? { 24'd0, rdcmd1 } :
                      (re_qspi_latency_dly[6] == 1'b1) ? { 24'd0, wrcmd0 } :
                      (re_qspi_latency_dly[7] == 1'b1) ? { 24'd0, wrcmd1 } :
                      (re_qspi_latency_dly[8] == 1'b1) ? { 26'd0, rdwrch } : dma_io_rdata_in;

wire [3:0] read_latency = ce_1_dec ? read_latency_1 :
                          ce_2_dec ? read_latency_2 : read_latency_0;

// read wait counter
reg [3:0] rwait_cntr;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 rwait_cntr <= 4'd0;
	else if (~state_rdwt & next_state_rdwt)
		 rwait_cntr <= read_latency;
	else if (rwait_cntr == 4'd0)
		 rwait_cntr <= 4'd0;
	else if (fall_edge)
		 rwait_cntr <= rwait_cntr - 4'd1;
end

assign read_wait_end = state_rdwt & (rwait_cntr == 4'd0) & fall_edge;

// read end counter
wire [3:0] read_length = word_w ? 4'd7 :
                         word_hw ? 4'd3 : 4'd1;

reg [3:0] read_cntr;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 read_cntr <= 4'd0;
	else if (~state_read & next_state_read)
		 read_cntr <= read_length;
	else if (read_cntr == 4'd0)
		 read_cntr <= 4'd0;
	else if (fall_edge)
		 read_cntr <= read_cntr - 4'd1;
end

assign read_data_end = state_read & (read_cntr == 4'd0) & fall_edge;

// inner i/f state machine
`define IN_IDLE  2'b00
`define IN_READ  2'b01
`define IN_WRITE  2'b10
reg [1:0] inner_state;

function [1:0] inner_machine;
input [1:0] inner_state;
input read_req;
input write_req;
input read_data_end;
input write_data_end;

begin
	case(inner_state)
		`IN_IDLE: if (read_req) inner_machine = `IN_READ;
				  else if (write_req) inner_machine = `IN_WRITE;
				  else inner_machine = `IN_IDLE;
		`IN_READ: if (read_data_end) inner_machine = `IN_IDLE;
				  else inner_machine = `IN_READ;
		`IN_WRITE: if (write_data_end) inner_machine = `IN_IDLE;
				  else inner_machine = `IN_WRITE;
		default : inner_machine = `IN_IDLE;
	endcase
end
endfunction

wire [1:0] next_inner_state = inner_machine( inner_state,
											   read_req,
											   write_req,
											   read_data_end,
											   write_data_end);

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		inner_state <= 2'b00;
	else
		inner_state <= next_inner_state;
end

assign cmd_freadq = (inner_state == `IN_READ);
assign cmd_qwrite = (inner_state == `IN_WRITE);
assign read_valid = read_data_end;
assign write_finish = write_data_end;

// input signal sampler

wire word_w_pre = read_req ? read_w : write_w;
wire word_hw_pre = read_req ? read_hw : write_hw;
wire [25:0] word_adr_pre = read_req ? read_adr[25:0] : write_adr[25:0];

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n) begin
		word_w <= 1'b0;
		word_hw <= 1'b0;
		word_adr <= 26'd0;
	end
	else if (read_req | write_req) begin
		word_w <= word_w_pre;
		word_hw <= word_hw_pre;
		word_adr <= word_adr_pre;
	end
end


always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		write_data_lat <= 32'd0;
	else if (write_req)
		write_data_lat <= write_data;
end


endmodule
