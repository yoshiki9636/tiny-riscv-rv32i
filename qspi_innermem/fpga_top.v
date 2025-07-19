/*
 * My RISC-V RV32I CPU
 *   FPGA Top module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module fpga_top (
	input clkin,
	input rst_n,

	input interrupt_0,

	input rx,
	output tx,
	output [2:0] rgb_led,
	inout [3:0] gpio,
	input gpi_in,

	input [1:0] init_latency,
	input init_cpu_start,
	input [1:0] init_uart

	);

wire clk;
wire locked;
// cpu
wire [31:0] pc_data; // input NAI!
wire cpu_start; // input
wire quit_cmd; // output NAI!
wire [31:2] cpu_start_adr;
wire i_read_req; // output
wire i_read_w; // output
wire i_read_hw; // output
wire [31:0] i_read_adr; // output
wire d_read_req; // output
wire d_read_w; // output
wire d_read_hw; // output
wire read_valid; // input
wire [31:0] d_read_adr; // output
wire [31:0] read_data; // input
wire d_write_req; // output
wire d_write_w; // output
wire d_write_hw; // output
wire write_finish; // input
wire [31:0] d_write_adr; // output
wire [31:0] d_write_data; // output

// qspi
wire read_req; // output
wire read_w; // output
wire read_hw; // output
wire [31:0] read_adr; // output
wire write_req; // output
wire write_w; // output
wire write_hw; // output
wire [31:0] write_adr; // output
wire [31:0] write_data; // output

// uart
wire u_read_req; // input
wire u_read_w; // input
wire [31:0] u_read_adr; // input
wire u_write_req; // input
wire u_write_w; // input
wire [31:0] u_write_adr; // input
wire [31:0] u_write_data; // input

// uart IO
wire [7:0] uart_io_char; // input
wire uart_io_we; // input
wire uart_io_full; // output
wire [15:0] uart_term;
wire rout_en;
wire [7:0] rout;
wire cpu_run_state;

// io bus for cpu
wire dma_io_we_c; // output
wire [15:2] dma_io_wadr_c; // output
wire [31:0] dma_io_wdata_c; // output
wire [15:2] dma_io_radr_c; // output
wire dma_io_radr_en_c; // output
// io bus for uart
wire dma_io_we_u; // output
wire [15:2] dma_io_wadr_u; // output
wire [31:0] dma_io_wdata_u; // output
wire [15:2] dma_io_radr_u; // output
wire dma_io_radr_en_u; // output
// io bus read datat
wire [31:0] dma_io_rdata; // input
wire [31:0] dma_io_rdata_in = 32'hdeadbeef; // input
//wire [31:0] dma_io_rdata_in = 32'd0; // input
wire [31:0] dma_io_rdata_in_2; // input
wire [31:0] dma_io_rdata_in_3; // input
wire [31:0] dma_io_rdata_in_4; // input
wire [31:0] dma_io_rdata_in_5; // input

// for free run counter signals
wire csr_mtie;
wire frc_cntr_val_leq;
//wire interrupt_clear;
wire ext_uart_interrpt_1shot;
wire csr_meie;
wire g_interrupt_1shot;
wire g_interrupt;

// io bus logics
wire dma_io_we = dma_io_we_c | dma_io_we_u;
wire [15:2] dma_io_wadr = dma_io_we_u ? dma_io_wadr_u : dma_io_wadr_c;
wire [31:0] dma_io_wdata = dma_io_we_u ? dma_io_wdata_u : dma_io_wdata_c;
wire dma_io_radr_en = dma_io_radr_en_c | dma_io_radr_en_u;
wire [15:2] dma_io_radr = dma_io_radr_en_u ? dma_io_radr_u : dma_io_radr_c;


clk_wiz_0 clk_wiz_0 (
	.clk_out1(clk),
	.reset(~rst_n),
	.locked(locked),
	.clk_in1(clkin)
	);

cpu_top cpu_top (
	.clk(clk),
	.rst_n(rst_n),
	.cpu_start(cpu_start),
	.init_cpu_start(init_cpu_start),
	.quit_cmd(quit_cmd),
	.cpu_start_adr(cpu_start_adr),
	.pc_data(pc_data),
	.csr_mtie(csr_mtie),
	.frc_cntr_val_leq(frc_cntr_val_leq),
	.cpu_run_state(cpu_run_state),
	.csr_meie(csr_meie),
	.g_interrupt_1shot(g_interrupt_1shot),
	.g_interrupt(g_interrupt),

	.i_read_req(i_read_req),
	.i_read_w(i_read_w),
	.i_read_hw(i_read_hw),
	.i_read_adr(i_read_adr),
	.d_read_req(d_read_req),
	.d_read_w(d_read_w),
	.d_read_hw(d_read_hw),
	.read_valid(read_valid),
	.d_read_adr(d_read_adr),
	.read_data(read_data),
	.d_write_req(d_write_req),
	.d_write_w(d_write_w),
	.d_write_hw(d_write_hw),
	.write_finish(write_finish),
	.d_write_adr(d_write_adr),
	.d_write_data(d_write_data),
	.dma_io_we(dma_io_we_c),
	.dma_io_wadr(dma_io_wadr_c),
	.dma_io_wdata(dma_io_wdata_c),
	.dma_io_radr(dma_io_radr_c),
	.dma_io_radr_en(dma_io_radr_en_c),
	.dma_io_rdata(dma_io_rdata)
	);

bus_gather bus_gather (
	.clk(clk),
	.rst_n(rst_n),
	.i_read_req(i_read_req),
	.i_read_w(i_read_w),
	.i_read_hw(i_read_hw),
	.i_read_adr(i_read_adr),
	.d_read_req(d_read_req),
	.d_read_w(d_read_w),
	.d_read_hw(d_read_hw),
	.d_read_adr(d_read_adr),
	.d_write_req(d_write_req),
	.d_write_w(d_write_w),
	.d_write_hw(d_write_hw),
	.d_write_adr(d_write_adr),
	.d_write_data(d_write_data),
	.u_read_req(u_read_req),
	.u_read_w(u_read_w),
	.u_read_adr(u_read_adr),
	.u_write_req(u_write_req),
	.u_write_w(u_write_w),
	.u_write_adr(u_write_adr),
	.u_write_data(u_write_data),
	.read_req(read_req),
	.read_w(read_w),
	.read_hw(read_hw),
	.read_adr(read_adr),
	.write_req(write_req),
	.write_w(write_w),
	.write_hw(write_hw),
	.write_adr(write_adr),
	.write_data(write_data)
	);

uart_top uart_top (
	.clk(clk),
	.rst_n(rst_n),
	.rx(rx),
	.tx(tx),
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
	.dma_io_we(dma_io_we_u),
	.dma_io_wadr(dma_io_wadr_u),
	.dma_io_wdata(dma_io_wdata_u),
	.dma_io_radr(dma_io_radr_u),
	.dma_io_radr_en(dma_io_radr_en_u),
	.dma_io_rdata_in(dma_io_rdata),
	.pc_data(pc_data),
	.cpu_start(cpu_start),
	.quit_cmd(quit_cmd),
	.start_adr(cpu_start_adr),
	.uart_io_char(uart_io_char),
	.uart_io_we(uart_io_we),
	.uart_io_full(uart_io_full),
	.uart_term(uart_term),
	.rout_en(rout_en),
	.rout(rout)
	);


qspi_innermem qspi_innermem (
	.clk(clk),
	.rst_n(rst_n),
	.init_latency(init_latency),
	.read_req(read_req),
	.read_w(read_w),
	.read_hw(read_hw),
	.read_valid(read_valid),
	.read_adr(read_adr),
	.read_data(read_data),
	.write_req(write_req),
	.write_w(write_w),
	.write_hw(write_hw),
	.write_finish(write_finish),
	.write_adr(write_adr),
	.write_data(write_data),
	.dma_io_we(dma_io_we),
	.dma_io_wadr(dma_io_wadr),
	.dma_io_wdata(dma_io_wdata),
	.dma_io_radr(dma_io_radr),
	.dma_io_radr_en(dma_io_radr_en),
	.dma_io_rdata_in(dma_io_rdata_in_4),
	.dma_io_rdata(dma_io_rdata_in_5)
	);

io_led io_led (
	.clk(clk),
	.rst_n(rst_n),
	.dma_io_we(dma_io_we),
	.dma_io_wadr(dma_io_wadr),
	.dma_io_wdata(dma_io_wdata),
	.dma_io_radr(dma_io_radr),
	.dma_io_radr_en(dma_io_radr_en),
	.dma_io_rdata_in(dma_io_rdata_in_2),
	.dma_io_rdata(dma_io_rdata_in_3),
	.rgb_led(rgb_led),
	.init_uart(init_uart),
	.init_latency(init_latency),
	.init_cpu_start(init_cpu_start),
	.gpi_in(gpi_in),
	.gpio(gpio)
	);

io_uart_out io_uart_out (
	.clk(clk),
	.rst_n(rst_n),
	.dma_io_we(dma_io_we),
	.dma_io_wadr(dma_io_wadr),
	.dma_io_wdata(dma_io_wdata),
	.dma_io_radr(dma_io_radr),
	.dma_io_radr_en(dma_io_radr_en),
	.dma_io_rdata_in(dma_io_rdata_in),
	.dma_io_rdata(dma_io_rdata_in_2),
	.uart_io_char(uart_io_char),
	.uart_io_we(uart_io_we),
	.uart_io_full(uart_io_full),
	.init_uart(init_uart),
	.uart_term(uart_term),
	.cpu_run_state(cpu_run_state),
	.rout_en(rout_en),
	.rout(rout),
	.ext_uart_interrpt_1shot(ext_uart_interrpt_1shot)
	);


io_frc io_frc (
	.clk(clk),
	.rst_n(rst_n),
	.dma_io_we(dma_io_we),
	.dma_io_wadr(dma_io_wadr),
	.dma_io_wdata(dma_io_wdata),
	.dma_io_radr(dma_io_radr),
	.dma_io_radr_en(dma_io_radr_en),
	.dma_io_rdata_in(dma_io_rdata_in_3),
	.dma_io_rdata(dma_io_rdata_in_4),
	.csr_mtie(csr_mtie),
	.frc_cntr_val_leq(frc_cntr_val_leq)
	);

interrupter interrupter (
	.clk(clk),
	.rst_n(rst_n),
	.interrupt_0(interrupt_0),
	.ext_uart_interrpt_1shot(ext_uart_interrpt_1shot),
	.csr_meie(csr_meie),
	.g_interrupt_1shot(g_interrupt_1shot),
	.g_interrupt(g_interrupt),
	.dma_io_we(dma_io_we),
	.dma_io_wadr(dma_io_wadr),
	.dma_io_wdata(dma_io_wdata),
	.dma_io_radr(dma_io_radr),
	.dma_io_radr_en(dma_io_radr_en),
	.dma_io_rdata_in(dma_io_rdata_in_5),
	.dma_io_rdata(dma_io_rdata)
	);

endmodule
