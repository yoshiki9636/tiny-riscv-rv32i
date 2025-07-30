/*
 * My RISC-V RV32I CPU
 *   QSPI psram simulation model
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module qspi_psram_model (
	input clk, // for simulation
	input rst_n, // for simulation
	input sck,
	input ce_n,
	inout [3:0] sio

	);
	
`define CMD_FREADQ 8'hEB
`define CMD_QWIRTE 8'h38
`define CMD_RST_EN 8'h66
`define CMD_RESET  8'h99


// tristate buffer of sio
wire sio_out_enbl;
wire [3:0] sio_in;
wire [3:0] sio_out;

assign sio = sio_out_enbl ? sio_out : 4'hz;
assign sio_in = sio;

// input FF for meta-stable
reg sck_sync;
reg ce_n_sync;
reg ce_n_sync_dly;
reg [3:0] sio_in_sync;
reg [3:0] sio_in_sync_lat;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n) begin
		sck_sync <= 1'b1;
		ce_n_sync <= 1'b1;
		ce_n_sync_dly <= 1'b1;
		sio_in_sync <= 4'h0;
	end
	else begin
		sck_sync <= sck;
		ce_n_sync <= ce_n;
		ce_n_sync_dly <= ce_n_sync;
		sio_in_sync <= sio_in;
	end
end

wire rise_edge = sck & ~sck_sync;
wire fall_edge = ~sck & sck_sync;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		sio_in_sync_lat <= 4'h0;
	else if (rise_edge)
		sio_in_sync_lat <= sio_in_sync;
end


// main state machine

`define MN_IDLE  4'b0000
`define MN_CMD   4'b0001
`define MN_ADR   4'b0010
`define MN_WTDAT 4'b0011
`define MN_RDWIT 4'b0101
`define MN_RDDAT 4'b0111
`define MN_RSTEN 4'b1000
`define MN_RESET 4'b1001

reg [3:0] main_state;

function [3:0] main_machine;
input [3:0] main_state;
input cmd_end;
input adr_end;
input rst_end;
input ce_n_sync;
input cmd_freadq;
input cmd_qwrite;
input cmd_rst_en;
input read_wait_end;

begin
	case(main_state)
		`MN_IDLE: if (~ce_n_sync) main_machine = `MN_CMD;
				  else main_machine = `MN_IDLE;
		`MN_CMD: if (cmd_end & (cmd_freadq | cmd_qwrite)) main_machine = `MN_ADR;
				 else if (cmd_end & cmd_rst_en) main_machine = `MN_RSTEN;
				 else if (cmd_end) main_machine = `MN_IDLE;
				 else main_machine = `MN_CMD;
		`MN_ADR: if (adr_end & cmd_freadq) main_machine = `MN_RDWIT;
			     else if (adr_end & cmd_qwrite) main_machine = `MN_WTDAT;
				 else main_machine = `MN_ADR;
		`MN_WTDAT: if (ce_n_sync) main_machine = `MN_IDLE;
				   else main_machine = `MN_WTDAT;
		`MN_RDWIT: if (read_wait_end) main_machine = `MN_RDDAT;
				   else main_machine = `MN_RDWIT;
		`MN_RDDAT: if (ce_n_sync) main_machine = `MN_IDLE;
				   else main_machine = `MN_RDDAT;
		`MN_RSTEN: if (rst_end) main_machine = `MN_RESET;
				   else main_machine = `MN_RSTEN;
		`MN_RESET: if (rst_end) main_machine = `MN_IDLE;
				   else main_machine = `MN_RESET;
		default : main_machine = `MN_IDLE;
	endcase
end
endfunction

wire cmd_end;
wire adr_end;
wire rst_end;
wire cmd_freadq;
wire cmd_qwrite;
wire cmd_rst_en;
wire read_wait_end;

wire [3:0] next_main_state = main_machine( main_state,
										   cmd_end,
										   adr_end,
										   rst_end,
										   ce_n_sync,
										   cmd_freadq,
										   cmd_qwrite,
										   cmd_rst_en,
										   read_wait_end);

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		main_state <= 4'b0000;
	else if (rise_edge)
		main_state <= next_main_state;
end

wire state_cmd = (main_state == `MN_CMD);
wire next_state_cmd = (next_main_state == `MN_CMD);
wire state_adr = (main_state == `MN_ADR);
wire next_state_adr = (next_main_state == `MN_ADR);
wire state_write = (main_state == `MN_WTDAT);
wire next_state_write = (next_main_state == `MN_WTDAT);
wire state_rdwt = (main_state == `MN_RDWIT);
wire next_state_rdwt = (next_main_state == `MN_RDWIT);
wire state_read = (main_state == `MN_RDDAT);
wire next_state_read = (next_main_state == `MN_RDDAT);
wire state_rsten = (main_state == `MN_RSTEN);
wire next_state_rsten = (next_main_state == `MN_RSTEN);
wire state_rst = (main_state == `MN_RESET);
wire next_state_rst = (next_main_state == `MN_RESET);

// command sampler
reg [7:0] byte_cmd;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		byte_cmd <= 8'd0;
	else if (~state_cmd & next_state_cmd & rise_edge)
		byte_cmd <= { 7'd0, sio_in_sync[0] };
	else if (state_cmd & rise_edge & ~cmd_end)
		byte_cmd <= { byte_cmd[6:0], sio_in_sync[0] };
end

assign cmd_freadq = (byte_cmd == `CMD_FREADQ);
assign cmd_qwrite = (byte_cmd == `CMD_QWIRTE);
assign cmd_rst_en = (byte_cmd == `CMD_RST_EN);

// cmd counter
reg [3:0] cmd_cntr;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 cmd_cntr <= 4'd0;
	else if (~ce_n_sync & ce_n_sync_dly)
		 cmd_cntr <= 4'd8;
	else if (cmd_cntr == 4'd0)
		 cmd_cntr <= 4'd0;
	else if (rise_edge)
		 cmd_cntr <= cmd_cntr - 4'd1;
end

assign cmd_end = state_cmd & (cmd_cntr == 4'd0) & rise_edge;


// adr counter
reg [3:0] adr_cntr;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 adr_cntr <= 4'd0;
	else if (~state_adr & next_state_adr)
		 adr_cntr <= 4'd5;
	else if (adr_cntr == 4'd0)
		 adr_cntr <= 4'd0;
	else if (rise_edge)
		 adr_cntr <= adr_cntr - 4'd1;
end

assign adr_end = state_adr & (adr_cntr == 4'd0);

// reset cntr
reg [3:0] rst_cntr;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 rst_cntr <= 4'd0;
	else if ((~state_rsten & next_state_rsten) | (~state_rst & next_state_rst))
		 rst_cntr <= 4'd9;
	else if (rst_cntr == 4'd0)
		 rst_cntr <= 4'd0;
	else if (rise_edge)
		 rst_cntr <= rst_cntr - 4'd1;
end

assign rst_end = (state_rsten | state_rst) & (rst_cntr == 4'd0) & rise_edge;

// read wait cntr
reg [3:0] rwait_cntr;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		 rwait_cntr <= 4'd0;
	else if (~state_rdwt & next_state_rdwt)
		 rwait_cntr <= 4'd3; // causion!!
	else if (rwait_cntr == 4'd0)
		 rwait_cntr <= 4'd0;
	else if (rise_edge)
		 rwait_cntr <= rwait_cntr - 4'd1;
end

assign read_wait_end = state_rdwt & (rwait_cntr == 0);


// address sampler
reg [23:0] word_adr;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		word_adr <= 24'd0;
	else if (~state_adr & next_state_adr & rise_edge)
		//word_adr <= { 20'd0, sio_in_sync } ;
		word_adr <= 24'd0;
	else if ( state_adr & rise_edge & ~adr_end)
		word_adr <= { word_adr[19:0], sio_in_sync } ;
end

// real write memory address cntr
reg write_half_byte;
reg [15:0] write_byte_cntr;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		write_half_byte <= 1'b1;
	else if (state_write & rise_edge)
		write_half_byte <= ~write_half_byte;
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		write_byte_cntr <= 16'd0;
	else if (~state_write & next_state_write)
		write_byte_cntr <= word_adr[15:0];
	else if (state_write & ~write_half_byte & rise_edge)
		write_byte_cntr <= write_byte_cntr + 16'd1;
end

wire [16:0] write_address = { write_byte_cntr, write_half_byte };

// write data enable
wire ram_write_en = state_write & rise_edge;


// real read memory address cntr
reg read_half_byte;
reg [15:0] read_byte_cntr;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		read_half_byte <= 1'b1;
	//else if (state_read & fall_edge)
	else if (~ce_n_sync & state_read & fall_edge)
		read_half_byte <= ~read_half_byte;
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		read_byte_cntr <= 16'd0;
	else if (~state_read & next_state_read)
		read_byte_cntr <= word_adr[15:0];
	//else if (state_read & ~read_half_byte & fall_edge)
	else if (~ce_n_sync & state_read & ~read_half_byte & fall_edge)
		read_byte_cntr <= read_byte_cntr + 16'd1;
end

wire [16:0] read_address = { read_byte_cntr, read_half_byte };

assign sio_out_enbl = (next_state_read | state_read) & ~ce_n_sync;

// memory

qspi_1r1w qspi_1r1w (
	.clk(clk),
	.ram_radr(read_address),
	.ram_rdata(sio_out),
	.ram_wadr(write_address),
	.ram_wdata(sio_in_sync_lat),
	.ram_wen(ram_write_en)
	);

endmodule
