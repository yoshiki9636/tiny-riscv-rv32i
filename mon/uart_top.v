/*
 * My RISC-V RV32I CPU
 *   UART Monitor Top Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2021 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module uart_top
    #(parameter IWIDTH = 14,
      parameter DWIDTH = 14)
	(

	input clk,
	input rst_n,
	input rx,
	output tx,

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

	input [31:0] pc_data,
	
	output cpu_start,
	output quit_cmd,

	output [31:2] start_adr,
    input [7:0] uart_io_char,
    input uart_io_we,
    output uart_io_full

	);


wire [31:0] uart_data;
wire [31:0] rdata_snd;
wire [7:0] rout;
wire [7:0] rx_rdata;
wire [7:0] send_char;
wire [7:0] tx_wdata;

wire flushing_wq;
wire inst_address_set;
wire inst_data_en;
wire pgm_end_set;
wire pgm_start_set;
wire pgm_stop;
wire rdata_snd_start;
wire read_end_set;
wire read_start_set;
wire read_stop;
wire rout_en;
wire rx_fifo_dvalid;
wire rx_fifo_full;
wire rx_fifo_overrun;
wire rx_fifo_underrun;
wire rx_rden;
wire send_en;
wire start_step;
wire start_trush;
wire tx_fifo_full;
wire tx_fifo_overrun;
wire tx_fifo_underrun;
wire tx_wten;
wire write_address_set;
wire write_data_en;
wire pc_print;
wire pc_print_sel;

wire dump_running;
wire trush_running;
//wire data_en;
wire crlf_in;
wire [2:0] rx_fifo_rcntr;


uart_if uart_if (
	.clk(clk),
	.rst_n(rst_n),
	.rx(rx),
	.tx(tx),
	.rx_rden(rx_rden),
	.rx_rdata(rx_rdata),
	.rx_fifo_full(rx_fifo_full),
	.rx_fifo_dvalid(rx_fifo_dvalid),
	.rx_fifo_overrun(rx_fifo_overrun),
	.rx_fifo_underrun(rx_fifo_underrun),
	.tx_wdata(tx_wdata),
	.tx_wten(tx_wten),
	.tx_fifo_full(tx_fifo_full),
	.tx_fifo_overrun(tx_fifo_overrun),
	.tx_fifo_underrun(tx_fifo_underrun),
	.rx_fifo_rcntrs(rx_fifo_rcntr)
	);

uart_loop uart_loop (
	.clk(clk),
	.rst_n(rst_n),
	.rout(rout),
	.rout_en(rout_en),
	.send_char(send_char),
	.send_en(send_en),
	.rx_rden(rx_rden),
	.rx_rdata(rx_rdata),
	.rx_fifo_full(rx_fifo_full),
	.rx_fifo_dvalid(rx_fifo_dvalid),
	.rx_fifo_overrun(rx_fifo_overrun),
	.rx_fifo_underrun(rx_fifo_underrun),
	.tx_wdata(tx_wdata),
	.tx_wten(tx_wten),
	.tx_fifo_full(tx_fifo_full),
	.tx_fifo_overrun(tx_fifo_overrun),
	.tx_fifo_underrun(tx_fifo_underrun),
    .uart_io_char(uart_io_char),
    .uart_io_we(uart_io_we),
    .uart_io_full(uart_io_full)
	);


uart_rec_char uart_rec_char (
	.clk(clk),
	.rst_n(rst_n),
	.rout(rout),
	.rout_en(rout_en),
	.dump_running(dump_running),
	.trush_running(trush_running),
	.dcflush_running(dcflush_running),
	.uart_data(uart_data),
	.cpu_start(cpu_start),
	.write_address_set(write_address_set),
	.write_data_en(write_data_en),
	.read_start_set(read_start_set),
	.read_end_set(read_end_set),
	.read_stop(read_stop),
	.start_trush(start_trush),
	.start_step(start_step),
	.quit_cmd(quit_cmd),
	.pgm_start_set(pgm_start_set),
	.pgm_end_set(pgm_end_set),
	.pgm_stop(pgm_stop),
	.inst_address_set(inst_address_set),
	.inst_data_en(inst_data_en),
	.pc_print(pc_print),
	.pc_print_sel(pc_print_sel),
	.crlf_in(crlf_in)
	);

uart_send_char uart_send_char (
	.clk(clk),
	.rst_n(rst_n),
	.rdata_snd_start(rdata_snd_start),
	.rdata_snd(rdata_snd),
	.flushing_wq(flushing_wq),
	.send_char(send_char),
	.send_en(send_en),
	.tx_fifo_full(tx_fifo_full),
	.crlf_in(crlf_in)
	);

uart_logics uart_logics (
	.clk(clk),
	.rst_n(rst_n),
	.u_read_req(u_read_req),
	.u_read_w(u_read_w),
	.read_valid(read_valid),
	.u_read_adr(u_read_adr),
	.read_data(read_data),
	.u_write_req(u_write_req),
	.u_write_w(u_write_w),
	.write_finish(write_finish),
	.u_write_adr(u_write_adr),
	.u_write_data(u_write_data),
	.uart_data(uart_data),
	.start_adr(start_adr),
	.write_address_set(write_address_set),
	.write_data_en(write_data_en),
	.read_start_set(read_start_set),
	.read_end_set(read_end_set),
	.read_stop(read_stop),
	.rdata_snd_start(rdata_snd_start),
	.rdata_snd(rdata_snd),
	.flushing_wq(flushing_wq),
	.dump_running(dump_running),
	.start_trush(start_trush),
	.trush_running(trush_running),
	.start_step(start_step),
	.pgm_start_set(pgm_start_set),
	.pgm_end_set(pgm_end_set),
	.pgm_stop(pgm_stop),
	.inst_address_set(inst_address_set),
	.pc_print(pc_print),
	.pc_print_sel(pc_print_sel),
	.pc_data(pc_data),
	.inst_data_en(inst_data_en)
	);



endmodule 
