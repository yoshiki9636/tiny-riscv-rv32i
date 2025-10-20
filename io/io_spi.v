/*
 * My RISC-V RV32I CPU
 *   FPGA SPI Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module io_spi(
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
`define SYS_SPI_EXEC  14'h3C80
`define SYS_SPI_MODE  14'h3C81
`define SYS_SPI_SDIV  14'h3C82
`define SYS_SPI_COML  14'h3C83
`define SYS_SPI_COMV  14'h3C84
`define SYS_SPI_DATL  14'h3C85
`define SYS_SPI_DATX  12'hF22

wire we_spi_exec = dma_io_we      & (dma_io_wadr == `SYS_SPI_EXEC);
wire re_spi_exec = dma_io_radr_en & (dma_io_radr == `SYS_SPI_EXEC);

wire we_spi_mode = dma_io_we      & (dma_io_wadr == `SYS_SPI_MODE);
wire re_spi_mode = dma_io_radr_en & (dma_io_radr == `SYS_SPI_MODE);

wire we_spi_sdiv = dma_io_we      & (dma_io_wadr == `SYS_SPI_SDIV);
wire re_spi_sdiv = dma_io_radr_en & (dma_io_radr == `SYS_SPI_SDIV);

wire we_spi_coml = dma_io_we      & (dma_io_wadr == `SYS_SPI_COML);
wire re_spi_coml = dma_io_radr_en & (dma_io_radr == `SYS_SPI_COML);

wire we_spi_comv = dma_io_we      & (dma_io_wadr == `SYS_SPI_COMV);
wire re_spi_comv = dma_io_radr_en & (dma_io_radr == `SYS_SPI_COMV);

wire we_spi_datl = dma_io_we      & (dma_io_wadr == `SYS_SPI_DATL);
wire re_spi_datl = dma_io_radr_en & (dma_io_radr == `SYS_SPI_DATL);

wire we_spi_datx = dma_io_we      & (dma_io_wadr[15:4] == `SYS_SPI_DATX);
wire re_spi_datx = dma_io_radr_en & (dma_io_radr[15:4] == `SYS_SPI_DATX);


reg [3:0] spi_exec;
wire spi_run_finish;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        spi_exec <= 4'd0 ;
	else if ( spi_run_finish )
		spi_exec <= { spi_exec[3:1], 1'b0 };
	else if ( we_spi_exec )
		spi_exec <= dma_io_wdata[3:0];
end

wire spi_exec_run = spi_exec[0];
wire spi_exec_rdwt_mode = spi_exec[1];
wire spi_exec_intr_mode = spi_exec[2];
wire spi_exec_csn_out = spi_exec[3];

reg [26:0] spi_mode;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        spi_mode <= { 9'd0, 3'b001, 2'b01, 3'd0 } ;
	else if ( we_spi_mode )
		spi_mode <= { dma_io_wdata[26:24], dma_io_wdata[22:20], dma_io_wdata[18:16], dma_io_wdata[10:8], dma_io_wdata[5:4], dma_io_wdata[2:1], dma_io_wdata[0] };
end

assign spi_select_io = spi_mode[0];
wire spi_mode_cpha = spi_mode[1];
wire spi_mode_cpol = spi_mode[2];
wire spi_mode_cmd_bit_endian = spi_mode[3];
wire spi_mode_cmd_byte_endian = spi_mode[4];
wire spi_mode_dat_bit_endian = spi_mode[5];
wire spi_mode_dat_byte_endian = spi_mode[6];
wire spi_mode_dat_word_endian = spi_mode[7];
wire [2:0] spi_mode_sck_ph_mosi = spi_mode[10:8];
wire [2:0] spi_mode_sck_ph_miso = spi_mode[13:11];
wire [2:0] spi_mode_miso_lattmg = spi_mode[16:14];

reg [9:0] spi_sck_div;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        spi_sck_div <= 10'd0 ;
	else if ( we_spi_sdiv )
		spi_sck_div <= dma_io_wdata[9:0];
end

reg [2:0] spi_cmd_len;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        spi_cmd_len <= 3'd1 ;
	else if ( we_spi_coml )
		spi_cmd_len <= dma_io_wdata[2:0];
end

reg [31:0] spi_cmd_val;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        spi_cmd_val <= 32'd0 ;
	else if ( we_spi_comv )
		spi_cmd_val <= dma_io_wdata[31:0];
end

reg [4:0] spi_dat_len;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        spi_dat_len <= 5'd0 ;
	else if ( we_spi_datl )
		spi_dat_len <= dma_io_wdata[4:0];
end

// spi buffer
reg [1:0] spi_dma_io_radr_dly;
reg [6:0] re_spi_value_dly;
wire re_spi_datx_dly = re_spi_value_dly[6];

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        spi_dma_io_radr_dly <= 2'd0 ;
	else
        spi_dma_io_radr_dly <= dma_io_radr[3:2];
end

wire [1:0] dat_word_sel;
reg [1:0] miso_write_adr;
wire [31:0] miso_word;
reg miso_write_current_word;

wire [1:0] spi_buf_radr = re_spi_datx_dly ? spi_dma_io_radr_dly : dat_word_sel;
wire [31:0] spi_buf_rdata;
wire [1:0] spi_buf_wadr = we_spi_datx ? dma_io_wadr[3:2] : miso_write_adr;
wire [31:0] spi_buf_wdata = we_spi_datx ? dma_io_wdata : miso_word;
wire [31:0] spi_buf_wen = we_spi_datx | miso_write_current_word;

wire [31:0] spi_dat_val = spi_buf_rdata;

spi_buf_0cycle spi_buf_0cycle(
	.clk(clk),
	.ram_radr(spi_buf_radr),
	.ram_rdata(spi_buf_rdata),
	.ram_wadr(spi_buf_wadr),
	.ram_wdata(spi_buf_wdata),
	.ram_wen(spi_buf_wen)
	);

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        re_spi_value_dly <= 7'd0 ;
    else
        re_spi_value_dly <= { re_spi_datx, re_spi_datl, re_spi_comv, re_spi_coml, re_spi_sdiv, re_spi_mode, re_spi_exec };
end

assign dma_io_rdata = (re_spi_value_dly[0] == 1'b1) ? { 28'd0, spi_exec } :
                      (re_spi_value_dly[1] == 1'b1) ? { 5'd0, spi_mode[16:14], 1'b0, spi_mode[13:11], 1'b0, spi_mode[10:8], 5'd0, spi_mode[7:5], 2'd0, spi_mode[4:3], 1'b0, spi_mode[2:0] } :

                      (re_spi_value_dly[2] == 1'b1) ? { 22'd0, spi_sck_div } :

                      (re_spi_value_dly[3] == 1'b1) ? { 29'd0, spi_cmd_len } :
                      (re_spi_value_dly[4] == 1'b1) ? spi_cmd_val :
                      (re_spi_value_dly[5] == 1'b1) ? { 27'd0, spi_dat_len } :
                      (re_spi_value_dly[6] == 1'b1) ? spi_dat_val : dma_io_rdata_in;

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

// bit select counter

reg [2:0] bit_sel_org;
wire write_cmd_status;
wire write_data_status;
wire read_cmd_status;
wire read_data_status;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        bit_sel_org <= 3'd0 ;
	else if ( ~write_data_status & ~read_cmd_status & ~write_cmd_status & ~spi_run_finish )
        bit_sel_org <= 3'd0 ;
	else if ( sck_write_lat_timing )
        bit_sel_org <= bit_sel_org + 3'd1 ;
end

wire [2:0] cmd_bit_sel = (spi_mode_cmd_bit_endian) ? ~bit_sel_org : bit_sel_org;
wire [2:0] dat_bit_sel = (spi_mode_dat_bit_endian) ? ~bit_sel_org : bit_sel_org;

//wire mosi_next_byte = (bit_sel_org == 3'd7) & sck_write_lat_timing;
wire mosi_next_byte = (bit_sel_org == 3'd7);

// byte select counter

reg [1:0] byte_sel_org;
wire start_cmd;
wire start_wirte;
wire start_read;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        byte_sel_org <= 2'd0 ;
	else if ((start_cmd | start_wirte | start_read) & sck_write_lat_timing )
        byte_sel_org <= 2'd0 ;
	else if ( mosi_next_byte & sck_write_lat_timing )
        byte_sel_org <= byte_sel_org + 2'd1 ;
end

wire [1:0] cmd_byte_sel = (spi_mode_cmd_byte_endian) ? ~byte_sel_org : byte_sel_org;
wire [1:0] dat_byte_sel = (spi_mode_dat_byte_endian) ? ~byte_sel_org : byte_sel_org;

reg [1:0] word_sel_org;
wire [4:0] spi_dat_len_m1 = spi_dat_len - 5'd1;
wire [2:0] spi_cmd_len_m1 = spi_cmd_len - 3'd1;

wire both_cmd_end = mosi_next_byte & (byte_sel_org == spi_cmd_len_m1[1:0]) ;
wire mosi_dat_end = mosi_next_byte & ( {word_sel_org, byte_sel_org} >= spi_dat_len_m1[3:0] );
wire mosi_next_word = sck_write_lat_timing & ( mosi_dat_end | (mosi_next_byte & (byte_sel_org == 3'd3)));

// word select counter 


always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        word_sel_org <= 2'd0 ;
	else if ((start_cmd | start_wirte | start_read) & sck_write_lat_timing )
        word_sel_org <= 2'd0 ;
	else if ( mosi_next_word & sck_write_lat_timing )
        word_sel_org <= word_sel_org + 2'd1 ;
end

assign dat_word_sel = (spi_mode_dat_word_endian) ? ~word_sel_org : word_sel_org;

// command byte selector

wire [7:0] cmd_byte = (cmd_byte_sel == 2'd0) ? spi_cmd_val[7:0] :
                      (cmd_byte_sel == 2'd1) ? spi_cmd_val[15:8] :
                      (cmd_byte_sel == 2'd2) ? spi_cmd_val[23:16] : spi_cmd_val[31:24];

// command bit selector

wire cmd_bit = cmd_byte[cmd_bit_sel];

// data byte selector

wire [7:0] data_byte = (dat_byte_sel == 2'd0) ? spi_buf_rdata[7:0] :
                       (dat_byte_sel == 2'd1) ? spi_buf_rdata[15:8] :
                       (dat_byte_sel == 2'd2) ? spi_buf_rdata[23:16] : spi_buf_rdata[31:24];

// data bit selector

wire data_bit = data_byte[dat_bit_sel];

// miso final selector
wire cmd_data_cmd_sel;
wire mosi_en;

wire org_mosi_pre = cmd_data_cmd_sel ? cmd_bit : data_bit;
wire org_mosi = org_mosi_pre & mosi_en;

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

assign spi_csn = (~cs_org) ? 2'b11 :
                 (spi_exec_csn_out) ? 2'b01 : 2'b10;

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
	else if ( ~read_data_status )
        miso_bit_cntr <= 3'd0 ;
	else if ( sck_read_lat_timing )
        miso_bit_cntr <= miso_bit_cntr + 3'd1 ;
end

wire miso_read_next_byte = (miso_bit_cntr == 3'd7) & sck_read_lat_timing;

// miso byte maker

reg [7:0] miso_byte_org;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        miso_byte_org <= 8'd0 ;
	else if ( ~read_data_status )
        miso_byte_org <= 8'd0 ;
	else if ( spi_dat_len == 5'd0 )
        miso_byte_org <= 8'd0 ;
	else if ( sck_read_lat_timing )
		miso_byte_org <= { miso_byte_org[7:0], miso_sel };
end

wire [7:0] miso_byte = (spi_mode_dat_bit_endian) ? miso_byte_org :
                       { miso_byte_org[0], miso_byte_org[1], miso_byte_org[2], miso_byte_org[3],
                         miso_byte_org[4], miso_byte_org[5], miso_byte_org[6], miso_byte_org[7] };


// byte counter
reg [3:0] miso_byte_cntr;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        miso_byte_cntr <= 4'd0 ;
	else if ( ~read_data_status )
        miso_byte_cntr <= 4'd0 ;
	else if ( spi_dat_len == 5'd0 )
        miso_byte_cntr <= 4'd0 ;
	else if (( miso_byte_cntr >= spi_dat_len_m1[3:0] ) & miso_read_next_byte )
        miso_byte_cntr <= 4'd0 ;
	else if ( miso_read_next_byte )
        miso_byte_cntr <= miso_byte_cntr + 4'd1 ;
end

wire miso_dat_end = ( miso_byte_cntr >= spi_dat_len_m1[3:0] ) & (miso_bit_cntr == 3'd7);

//assign miso_write_adr = spi_mode_dat_word_endian ? ~miso_byte_cntr[3:2] : miso_byte_cntr[3:2];
//assign miso_write_current_word = miso_read_next_byte & ( miso_dat_end | ( miso_byte_cntr[1:0] == 2'd3 ));
always @ (posedge clk or negedge rst_n) begin
    if (~rst_n) begin
        miso_write_current_word <= 1'b0 ;
        miso_write_adr <= 2'd0 ;
	end
	else begin
		miso_write_current_word <= miso_read_next_byte & ( miso_dat_end | ( miso_byte_cntr[1:0] == 2'd3 ));
		miso_write_adr <= spi_mode_dat_word_endian ? ~miso_byte_cntr[3:2] : miso_byte_cntr[3:2];
	end
end

// word maker
reg [31:0] miso_word_org;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        //miso_word_org <= 32'd0 ;
        miso_word_org <= 32'hdeadbeef ;
	else if ( miso_write_current_word )
        //miso_word_org <= 32'd0 ;
        miso_word_org <= 32'hdeadbeef ;
	else if ( miso_read_next_byte )
		miso_word_org <= { miso_word_org[23:0], miso_byte };
end

assign miso_word = (spi_mode_dat_byte_endian) ? { miso_word_org[7:0], miso_word_org[15:8], miso_word_org[23:16], miso_word_org[31:24] } : miso_word_org;


// state machine

`define SPI_IDLE  3'b000
`define SPI_WTCMD 3'b001
`define SPI_WTDAT 3'b010
`define SPI_PREWT 3'b011
`define SPI_RDCMD 3'b101
`define SPI_RDDAT 3'b110
`define SPI_WAIT  3'b111

reg [2:0] spi_state;

function [2:0] spi_machine;
input [2:0] spi_state;
input spi_exec_run;
input spi_exec_rdwt_mode;
input both_cmd_end;
input mosi_dat_end;
input miso_dat_end;
input spi_mode_cpha;
input spi_mode_cpol;


begin
	case(spi_state)
		`SPI_IDLE: if (spi_mode_cpha & spi_mode_cpol & spi_exec_run & spi_exec_rdwt_mode ) spi_machine = `SPI_PREWT;
				   else if (spi_mode_cpha & spi_mode_cpol & spi_exec_run & ~spi_exec_rdwt_mode ) spi_machine = `SPI_PREWT;
		           else if (spi_exec_run & spi_exec_rdwt_mode ) spi_machine = `SPI_WTCMD;
				   else if (spi_exec_run & ~spi_exec_rdwt_mode ) spi_machine = `SPI_RDCMD;
                   else spi_machine = `SPI_IDLE;
		`SPI_PREWT:if ( spi_exec_rdwt_mode ) spi_machine = `SPI_WTCMD;
				   else spi_machine = `SPI_RDCMD;
		`SPI_WTCMD:if ( ~spi_exec_run ) spi_machine = `SPI_IDLE;
                   else if ( both_cmd_end ) spi_machine = `SPI_WTDAT;
                   else spi_machine = `SPI_WTCMD;
		`SPI_WTDAT:if ( ~spi_exec_run ) spi_machine = `SPI_IDLE;
                   else if ( mosi_dat_end ) spi_machine = `SPI_WAIT;
                   else spi_machine = `SPI_WTDAT;
		`SPI_RDCMD:if ( ~spi_exec_run ) spi_machine = `SPI_IDLE;
                   else if ( both_cmd_end ) spi_machine = `SPI_RDDAT;
                   else spi_machine = `SPI_RDCMD;
		`SPI_RDDAT:if ( ~spi_exec_run ) spi_machine = `SPI_IDLE;
                   else if ( miso_dat_end ) spi_machine = `SPI_WAIT;
                   else spi_machine = `SPI_RDDAT;
		`SPI_WAIT: spi_machine = `SPI_IDLE;
		default : spi_machine = `SPI_IDLE;
	endcase
end
endfunction

wire [2:0] next_spi_state = spi_machine( spi_state,
										 spi_exec_run,
										 spi_exec_rdwt_mode,
										 both_cmd_end,
										 mosi_dat_end,
										 miso_dat_end,
										 spi_mode_cpha,
										 spi_mode_cpol);

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		spi_state <= 3'b000;
	//else if (sck_fall_edge)
	else if (sck_write_lat_timing)
		spi_state <= next_spi_state;
end


assign sck_en = (spi_state != `SPI_IDLE)&(spi_state != `SPI_WAIT)&~((next_spi_state == `SPI_WAIT)&sck_write_lat_timing);
assign sck_finish =  (spi_state == `SPI_WAIT)|((next_spi_state == `SPI_WAIT)&sck_write_lat_timing);
assign cs_all_status = (spi_state != `SPI_IDLE);
assign write_cmd_status = (spi_state == `SPI_WTCMD);
assign write_data_status = (spi_state == `SPI_WTDAT);
assign read_cmd_status = (spi_state == `SPI_RDCMD);
assign read_data_status = (spi_state == `SPI_RDDAT)|(spi_state == `SPI_WAIT);
assign start_cmd = ((next_spi_state == `SPI_WTCMD)|(next_spi_state == `SPI_RDCMD)) & (spi_state == `SPI_IDLE);
assign start_wirte = (next_spi_state == `SPI_WTDAT)&(spi_state == `SPI_WTCMD);
assign start_read = (next_spi_state == `SPI_RDDAT)&(spi_state == `SPI_RDCMD);
assign spi_run_finish =  (spi_state == `SPI_WAIT);
assign cmd_data_cmd_sel = (spi_state == `SPI_WTCMD)|(spi_state == `SPI_RDCMD);
assign mosi_en = write_cmd_status | write_data_status | read_cmd_status;

endmodule
