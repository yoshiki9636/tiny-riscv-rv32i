/*
 * My RISC-V RV32I CPU
 *   FPGA LED output Module for Tang Premier
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2024 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module io_uart_out(
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

	output reg [7:0] uart_io_char,
	output reg uart_io_we,
	input uart_io_full,
	input [1:0] init_uart,
	output reg [15:0] uart_term

	);

// decode :: adr 0x0 : LED values
`define SYS_UART_OUTC 14'h3F00
`define SYS_UART_FULL 14'h3F01
`define SYS_UART_TERM 14'h3F02

wire we_uart_char = dma_io_we      & (dma_io_wadr == `SYS_UART_OUTC);
wire re_uart_char = dma_io_radr_en & (dma_io_radr == `SYS_UART_OUTC);

wire re_uart_full = dma_io_radr_en & (dma_io_radr == `SYS_UART_FULL);

wire we_uart_term = dma_io_we      & (dma_io_wadr == `SYS_UART_TERM);
wire re_uart_term = dma_io_radr_en & (dma_io_radr == `SYS_UART_TERM);

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        uart_io_char <= 8'd0 ;
	else if ( we_uart_char )
		uart_io_char <= dma_io_wdata[7:0];
end

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        uart_io_we <= 1'b0 ;
	else
        uart_io_we <= we_uart_char & ~uart_io_full;
end

// UART term reset values
// these values should be changed for tapeout
// clk 100MHz, 921600bps
`define TERM_0 16'd109
// clk 50MHz, 921600bps
`define TERM_1 16'd54
// clk:50MHz, 9600bps
`define TERM_2 16'd5208
// clk:48MHz, 9600bps
`define TERM_3 16'd5000
	
wire [15:0] uart_term_reset_value = (init_uart == 2'd0) ? `TERM_0 :
                                    (init_uart == 2'd1) ? `TERM_1 :
                                    (init_uart == 2'd2) ? `TERM_2 : `TERM_3;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        uart_term <= uart_term_reset_value ;
	else if ( we_uart_term )
		uart_term <= dma_io_wdata[15:0];
end

// read part

reg [2:0] re_uart_rdflg_dly;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        re_uart_rdflg_dly <= 3'd0 ;
	else
        re_uart_rdflg_dly <= { re_uart_term, re_uart_full, re_uart_char };
end

assign dma_io_rdata = (re_uart_rdflg_dly[0]) ? { 24'd0, uart_io_char } :
                      (re_uart_rdflg_dly[1]) ? { 31'd0, uart_io_full } :
                      (re_uart_rdflg_dly[2]) ? { 16'd0, uart_term } : dma_io_rdata_in;


endmodule
