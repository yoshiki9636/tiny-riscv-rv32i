/*
 * My RISC-V RV32I CPU
 *   UART Monitor Logic Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2021 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module uart_logics (
	input clk,
	input rst_n,

    output u_read_req,
    output u_read_w,
    input read_valid,
    output [31:0] u_read_adr,
    input [31:0] read_data,

    output u_write_req,
    output u_write_w,
    input write_finish,
    output [31:0] u_write_adr,
    output [31:0] u_write_data,

	// from controller
	input [31:0] uart_data,
	output [31:2] start_adr,
	input write_address_set,
	input write_data_en,
	input read_start_set,
	input read_end_set,
	input read_stop,
	output rdata_snd_start,
	output [31:0] rdata_snd,
	input flushing_wq,
	output dump_running,
	input start_trush,
	output trush_running,
	input start_step,
	input pgm_start_set,
	input pgm_end_set,
	input pgm_stop,
	input inst_address_set,
	input pc_print,
	input pc_print_sel,
	input [31:0] pc_data,
	input inst_data_en

	);


// CPU running state
assign start_adr = uart_data[31:2];


// iram write address 
reg [31:2] cmd_wadr_cntr;
wire [21:2] trush_adr;
wire trash_req;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		cmd_wadr_cntr <= 30'd0;
	else if (write_address_set | inst_address_set)
		cmd_wadr_cntr <= uart_data[31:2];
	else if (write_data_en | inst_data_en)
		cmd_wadr_cntr <= cmd_wadr_cntr + 30'd1;
end

reg write_stat;

assign u_write_adr = trush_running ? { { 10{ 1'b0}}, trush_adr} : cmd_wadr_cntr[31:2];
assign u_write_data = trush_running ? 32'd0 : uart_data;
assign u_write_req = (write_data_en | trash_req) & ~write_stat; // trash req not work currently
assign u_write_w = 1'b1;
assign u_read_w = 1'b1;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		write_stat <= 1'b0;
	else if (write_finish)
		write_stat <= 1'b0;
	else if (u_write_req)
		write_stat <= 1'b1;
end



// ram read address
reg [31:2] cmd_read_end;
reg [32:2] cmd_read_adr;
reg dread_dsel;

wire dump_end;
wire radr_cntup;
wire dradr_cntup;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		cmd_read_adr <= 31'd0;
	else if (read_start_set | pgm_start_set)
		cmd_read_adr <= { 1'b0, uart_data[31:2] };
	else if (dradr_cntup)
		cmd_read_adr <= cmd_read_adr + 31'd2;
	else if (radr_cntup)
		cmd_read_adr <= cmd_read_adr + 31'd1;
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		dread_dsel <= 1'b0;
	else
		dread_dsel <= cmd_read_adr[3];
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		cmd_read_end <= 31'd0;
	else if (read_end_set | pgm_end_set)
		cmd_read_end <= uart_data[31:2];
end

assign dump_end =(cmd_read_adr >= {1'b0, cmd_read_end});

assign u_read_adr = { cmd_read_adr[31:2], 2'b00 };


`define D_IDLE 3'd0
`define D_RED1 3'd1
`define D_RED2 3'd2
`define D_DRWT 3'd3
`define D_DRDF 3'd4
`define D_WAIT 3'd5
reg [2:0] status_dump;
wire [2:0] next_status_dump;


function [2:0] dump_status;
input [2:0] status_dump;
input read_end_set;
input pgm_end_set;
input read_stop;
input pgm_stop;
input flushing_wq;
input dump_end;
input pc_print;
input pc_print_sel;
input read_valid;
begin
	case(status_dump)
		`D_IDLE :
			if (pgm_end_set)
				dump_status = `D_RED1;
			else if (read_end_set)
				dump_status = `D_DRWT;
			else if (pc_print)
				dump_status = `D_WAIT;
			else
				dump_status = `D_IDLE;
		`D_RED1 :
			if (pgm_stop)
				dump_status = `D_IDLE;
			else
				dump_status = `D_RED2;
		`D_RED2 :
			if (pgm_stop)
				dump_status = `D_IDLE;
			else
				dump_status = `D_WAIT;
		`D_DRWT :
			if (read_stop)
				dump_status = `D_IDLE;
			else if (read_valid)
				dump_status = `D_DRDF;
			else
				dump_status = `D_DRWT;
		`D_DRDF :
			if (read_stop | pgm_stop)
				dump_status = `D_IDLE;
			else if (flushing_wq & dump_end)
				dump_status = `D_IDLE;
			else if (flushing_wq & ~dump_end)
				dump_status = `D_DRWT;
			else
				dump_status = `D_DRDF;
		`D_WAIT :
			if (read_stop | pgm_stop)
				dump_status = `D_IDLE;
			else if (flushing_wq & pc_print_sel)
				dump_status = `D_IDLE;
			else if (flushing_wq & dump_end)
				dump_status = `D_IDLE;
			else if (flushing_wq & ~dump_end)
				dump_status = `D_RED1;
			else
				dump_status = `D_WAIT;
		default : dump_status = `D_IDLE;
	endcase
end
endfunction

assign next_status_dump = dump_status(
							status_dump,
							read_end_set,
							pgm_end_set,
							read_stop,
							pgm_stop,
							flushing_wq,
							dump_end,
							pc_print,
							pc_print_sel,
						    read_valid
							);

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		status_dump <= 3'd0;
	else
		status_dump <= next_status_dump;
end

assign radr_cntup = (status_dump == `D_RED1)|(status_dump == `D_RED2);
assign dradr_cntup = (status_dump == `D_DRWT)&(next_status_dump == `D_DRDF);
assign dread_start = ((status_dump == `D_IDLE)|(status_dump == `D_DRDF))&(next_status_dump == `D_DRWT);
assign dump_running = (status_dump != `D_IDLE);
wire rdata_snd_wait = (status_dump == `D_WAIT)|(status_dump == `D_DRDF);

reg i_ram_sel;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		i_ram_sel <= 1'b0;
	else if ( read_end_set )
		i_ram_sel <= 1'b0;
	else if ( pgm_end_set )
		i_ram_sel <= 1'b1;
end

//wire en0_data = radr_cntup | dradr_cntup;

reg [31:0] data_0;
 
always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		data_0 <= 32'd0;
	else if (read_valid)
		data_0 <= read_data;
end


assign rdata_snd = pc_print_sel ? pc_data : data_0;

// trashing memory data
reg [22:2] trash_cntr;
reg [22:2] trash_cntr_dly;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		trash_cntr <= { 21{ 1'b0 }};
	else if (start_trush)
		trash_cntr <= { 1'b1, { 21{ 1'b0 }}};
	else if (trash_cntr[22] & ~write_stat)
		trash_cntr <= trash_cntr + 1;
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		trash_cntr_dly <= { 21{ 1'b0 }};
	else
		trash_cntr_dly <= trash_cntr;
end

assign trush_adr = trash_cntr[21:2];
assign trush_running = trash_cntr[22];

assign trash_req = trush_running & (trash_cntr != trash_cntr_dly);

// send CPU status to UART i/f
/*
always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		cupst_snd_wait <= 1'b0;
	else if (flushing_wq)
		cupst_snd_wait <= 1'b0;
	else if (s04_wtbk)
		cupst_snd_wait <= 1'b1;
end

reg cpust_snd_wait_dly;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		cpust_snd_wait_dly <= 1'b0;
	else
		cpust_snd_wait_dly <= cupst_snd_wait;
end

assign cpust_start = cupst_snd_wait & ~cpust_snd_wait_dly;
*/

reg rdata_snd_wait_dly;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		rdata_snd_wait_dly <= 1'b0;
	else
		rdata_snd_wait_dly <= rdata_snd_wait;
end

assign rdata_snd_start = (rdata_snd_wait & ~rdata_snd_wait_dly) | pc_print;


endmodule
