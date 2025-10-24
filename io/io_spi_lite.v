/*
 * My RISC-V RV32I CPU
 *   FPGA SPI Module lite
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module io_spi_lite(
	input clk,
	input rst_n,
	// from/to IO bus

    input dma_io_we,
    input [15:2] dma_io_wadr,
    input [31:0] dma_io_wdata,
    input [15:2] dma_io_radr,
    input dma_io_radr_en,
    input [31:0] dma_io_rdata_in,
    output [31:0] dma_io_rdata,

	output spi_select_io,
	output spi_sck,
	output [1:0] spi_csn,
	output reg spi_mosi,
	input spi_miso
	);


// register & bus
`define SYS_SPI_MODE  14'h3C80
`define SYS_SPI_SDIV  14'h3C81
`define SYS_SPI_MOSI  14'h3C82
`define SYS_SPI_MISO  14'h3C83

wire we_spi_mode = dma_io_we      & (dma_io_wadr == `SYS_SPI_MODE);
wire re_spi_mode = dma_io_radr_en & (dma_io_radr == `SYS_SPI_MODE);

wire we_spi_sdiv = dma_io_we      & (dma_io_wadr == `SYS_SPI_SDIV);
wire re_spi_sdiv = dma_io_radr_en & (dma_io_radr == `SYS_SPI_SDIV);

wire we_spi_mosi = dma_io_we      & (dma_io_wadr == `SYS_SPI_MOSI);
wire re_spi_mosi = dma_io_radr_en & (dma_io_radr == `SYS_SPI_MOSI);

wire we_spi_miso = dma_io_we      & (dma_io_wadr == `SYS_SPI_MISO);
wire re_spi_miso = dma_io_radr_en & (dma_io_radr == `SYS_SPI_MISO);


reg [12:0] spi_mode;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        spi_mode <= { 9'd0, 1'b1, 3'd0 } ;
	else if ( we_spi_mode )
		spi_mode <= { dma_io_wdata[26:24], dma_io_wdata[22:20], dma_io_wdata[18:16], dma_io_wdata[3], dma_io_wdata[2:1], dma_io_wdata[0] };
end

assign spi_select_io = spi_mode[0];
wire spi_mode_cpha = spi_mode[1];
wire spi_mode_cpol = spi_mode[2];
wire spi_mode_bit_endian = spi_mode[3];
wire [2:0] spi_mode_sck_ph_mosi = spi_mode[6:4];
wire [2:0] spi_mode_sck_ph_miso = spi_mode[9:7];
wire [2:0] spi_mode_miso_lattmg = spi_mode[12:10];

reg [9:0] spi_sck_div;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        spi_sck_div <= 10'd0 ;
	else if ( we_spi_sdiv )
		spi_sck_div <= dma_io_wdata[9:0];
end

// miso signals
reg [3:0] re_spi_value_dly;

wire miso_fifo_re = re_spi_value_dly[3];
wire miso_reset_fifo = we_spi_miso & dma_io_wdata[10]; // need to add
wire [7:0] miso_fifo_out;
wire miso_fifo_full;
wire miso_fifo_empty;

// mosi signals
wire mosi_fifo_full;
wire mosi_fifo_empty;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        re_spi_value_dly <= 4'd0 ;
    else
        re_spi_value_dly <= { re_spi_miso, re_spi_mosi, re_spi_sdiv, re_spi_mode };
end

assign dma_io_rdata = (re_spi_value_dly[0] == 1'b1) ? { 5'd0, spi_mode[12:10], 1'b0, spi_mode[9:7], 1'b0, spi_mode[6:4], 12'd0, spi_mode[3:0] } :

                      (re_spi_value_dly[1] == 1'b1) ? { 22'd0, spi_sck_div } :

                      (re_spi_value_dly[2] == 1'b1) ? { 22'd0, mosi_fifo_empty, mosi_fifo_full, 8'd0 } :
                      (re_spi_value_dly[3] == 1'b1) ? { 22'd0, miso_fifo_empty, miso_fifo_full, miso_fifo_out } : dma_io_rdata_in;

// logics
// sck divide counter

reg [9:0] sck_div;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        sck_div <= 10'd0 ;
	else if ( sck_div == spi_sck_div )
        sck_div <= 10'd0 ;
	else
        sck_div <= sck_div + 10'd1 ;
end

wire chg_sck_edge = ( sck_div == spi_sck_div );

reg org_sck;
wire cs_all_status;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        org_sck <= 1'b0 ;
	//else if ( ~cs_all_status )
        //org_sck <= spi_mode_cpol; // sck polarity in no chip select
	else if ( chg_sck_edge )
        org_sck <= ~org_sck ;
end

reg org_sck_dly;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        org_sck_dly <= 1'b0 ;
	else
        org_sck_dly <= org_sck ;
end

// working points
wire sck_rise_edge =  org_sck & ~org_sck_dly;
wire sck_fall_edge = ~org_sck &  org_sck_dly;

wire sck_read_lat_timing = spi_mode_cpha ? sck_fall_edge : sck_rise_edge;
wire sck_write_lat_timing = spi_mode_cpha ? sck_rise_edge : sck_fall_edge;

// sck maker
wire sck_en;
wire sck_finish;

wire org_spi_sck = sck_en ? org_sck :
                   sck_finish ? 1'b0 : spi_mode_cpol;

// sck output shifter
reg [6:0] shift_sck;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        shift_sck <= 7'd0 ;
	else
		shift_sck <= { shift_sck[5:0], org_spi_sck };
end

wire [7:0] sel_sck = { shift_sck, org_spi_sck };

assign spi_sck = sel_sck[spi_mode_sck_ph_miso];

// mosi part
// mosi fifo

// fifo signals
wire [7:0] mosi_fifo_in = dma_io_wdata[7:0];
wire [7:0] mosi_fifo_out;
//wire mosi_fifo_full;
//wire mosi_fifo_empty;
wire mosi_fifo_we = we_spi_mosi & ~mosi_fifo_full;
wire mosi_fifo_re;
wire dummy_mosi_full;
wire dummy_mosi_empty;

// fifo module
sfifo_withr mosi_fifo (
	.clk(clk),
	.rst_n(rst_n),
	.wen(mosi_fifo_we),
	.wqfull(dummy_mosi_full),
	.wdata(mosi_fifo_in),
	.rnext(mosi_fifo_re),
	.rqempty(dummy_mosi_empty),
	.rdata(mosi_fifo_out),
	.q_reset(1'b0)
	);

// mosi fifo push/pull counter
reg [3:0] mosi_pp_cntr;
wire mosi_next_byte;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        mosi_pp_cntr <= 4'd0 ;
    else if ( mosi_fifo_we & mosi_fifo_re )
        mosi_pp_cntr <= mosi_pp_cntr;
    else if ( mosi_fifo_we )
        mosi_pp_cntr <= mosi_pp_cntr + 4'd1 ;
    else if ( mosi_fifo_re )
        mosi_pp_cntr <= mosi_pp_cntr - 4'd1 ;
end

assign mosi_fifo_empty = (mosi_pp_cntr == 4'd0);
assign mosi_fifo_full = (mosi_pp_cntr == 4'd8);

wire mosi_fifo_next_empty = (mosi_pp_cntr == 4'd1) & mosi_next_byte;


// bit select counter

reg [2:0] bit_sel_org;
wire data_phase;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        bit_sel_org <= 3'd0 ;
	else if ( ~data_phase )
        bit_sel_org <= 3'd0 ;
	else if ( sck_write_lat_timing )
        bit_sel_org <= bit_sel_org + 3'd1 ;
end

wire [2:0] dat_bit_sel = (spi_mode_bit_endian) ? ~bit_sel_org : bit_sel_org;

assign mosi_next_byte = (bit_sel_org == 3'd7);

wire mosi_fifo_next = mosi_next_byte & ~mosi_fifo_next_empty;
wire mosi_fifo_end  = mosi_next_byte &  mosi_fifo_next_empty;

assign mosi_fifo_re = (mosi_fifo_next | mosi_fifo_end) & sck_write_lat_timing;

// data bit selector
wire data_bit = mosi_fifo_out[dat_bit_sel];

// miso final selector
wire mosi_en;

wire org_mosi = data_bit & mosi_en;


// output shifter

reg [6:0] shift_mosi;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        shift_mosi <= 7'd0 ;
	else
		shift_mosi <= { shift_mosi[5:0], org_mosi };
end

wire [7:0] sel_mosi = { shift_mosi, org_mosi };

wire spi_mosi_pre = sel_mosi[spi_mode_sck_ph_mosi];

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        spi_mosi <= 1'b0 ;
	else
		spi_mosi <= spi_mosi_pre;
end

// CS output shifter

reg [6:0] shift_cs;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        shift_cs <= 7'd0 ;
	else
		shift_cs <= { shift_cs[5:0], cs_all_status };
end

wire [7:0] sel_cs = { shift_cs, cs_all_status };

wire cs_org = sel_cs[spi_mode_sck_ph_mosi];

assign spi_csn = (~cs_org) ? 2'b11 : 2'b10;


// miso input latch
reg [7:0] miso_lat;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        miso_lat <= 8'd0 ;
	else
		miso_lat <= { miso_lat[6:0], spi_miso };
end

wire miso_sel = miso_lat[spi_mode_miso_lattmg];

// bit counter
reg [2:0] miso_bit_cntr;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        miso_bit_cntr <= 3'd0 ;
	else if ( ~data_phase )
        miso_bit_cntr <= 3'd0 ;
	else if ( sck_read_lat_timing )
        miso_bit_cntr <= miso_bit_cntr + 3'd1 ;
end

wire miso_read_next_byte = (miso_bit_cntr == 3'd7) & sck_read_lat_timing;

reg miso_wen_byte;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        miso_wen_byte <= 1'b0 ;
	else
		miso_wen_byte <= miso_read_next_byte;
end

// miso byte maker

reg [7:0] miso_byte_org;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        miso_byte_org <= 8'd0 ;
	else if ( ~data_phase )
        miso_byte_org <= 8'd0 ;
	else if ( sck_read_lat_timing )
		miso_byte_org <= { miso_byte_org[7:0], miso_sel };
end

wire [7:0] miso_byte = (spi_mode_bit_endian) ? miso_byte_org :
                       { miso_byte_org[0], miso_byte_org[1], miso_byte_org[2], miso_byte_org[3],
                         miso_byte_org[4], miso_byte_org[5], miso_byte_org[6], miso_byte_org[7] };

// miso fifo
wire [7:0] miso_fifo_in = miso_byte;
//wire [7:0] miso_fifo_out;
//wire miso_fifo_full;
//wire miso_fifo_empty;
wire miso_fifo_we = miso_wen_byte & ~miso_fifo_full;
//wire miso_fifo_re;
//wire miso_reset_fifo; // need to add

sfifo_withr miso_fifo (
	.clk(clk),
	.rst_n(rst_n),
	.wen(miso_fifo_we),
	.wqfull(miso_fifo_full),
	.wdata(miso_fifo_in),
	.rnext(miso_fifo_re),
	.rqempty(miso_fifo_empty),
	.rdata(miso_fifo_out),
	.q_reset(miso_reset_fifo)
	);

// state machine

`define SPI_IDLE  3'b000
`define SPI_PREWT 3'b001
`define SPI_TRNSD 3'b010
`define SPI_WAIT  3'b011

reg [2:0] spi_state;

function [2:0] spi_machine;
input [2:0] spi_state;
input mosi_fifo_empty;
input mosi_fifo_next;
input mosi_fifo_end;

begin
	case(spi_state)
		`SPI_IDLE: if (spi_mode_cpha & spi_mode_cpol & ~mosi_fifo_empty ) spi_machine = `SPI_PREWT;
		           else if (~mosi_fifo_empty ) spi_machine = `SPI_TRNSD;
                   else spi_machine = `SPI_IDLE;
		`SPI_PREWT: spi_machine = `SPI_TRNSD;
		`SPI_TRNSD:if ( mosi_fifo_next ) spi_machine = `SPI_TRNSD;
                   else if ( mosi_fifo_end ) spi_machine = `SPI_WAIT;
                   else spi_machine = `SPI_TRNSD;
		`SPI_WAIT: spi_machine = `SPI_IDLE;
		default : spi_machine = `SPI_IDLE;
	endcase
end
endfunction

wire [2:0] next_spi_state = spi_machine( spi_state,
										 mosi_fifo_empty,
										 mosi_fifo_next,
										 mosi_fifo_end);

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		spi_state <= 3'b000;
	else if (sck_write_lat_timing)
		spi_state <= next_spi_state;
end


assign sck_en = (spi_state != `SPI_IDLE)&(spi_state != `SPI_WAIT)&~((next_spi_state == `SPI_WAIT)&sck_write_lat_timing);
assign sck_finish =  (spi_state == `SPI_WAIT)|((next_spi_state == `SPI_WAIT)&sck_write_lat_timing);
assign cs_all_status = (spi_state != `SPI_IDLE);
assign data_phase = (spi_state == `SPI_TRNSD);
assign mosi_en = (spi_state == `SPI_PREWT)|(spi_state == `SPI_TRNSD);


endmodule
