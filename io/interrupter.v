/*
 * My RISC-V RV32I CPU
 *   CPU Interrupter
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2023 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module interrupter(
	input clk,
	input rst_n,
	// from external
	input interrupt_0,
	// from clear I/O ( temporary in i/o FRC block)
	//input interrupt_clear,
	input ext_uart_interrpt_1shot,
	// from csr
	input csr_meie,
	input csr_rmie,
	output g_interrupt_1shot,
	output g_interrupt,

	input dma_io_we,
	input [15:2] dma_io_wadr,
	input [31:0] dma_io_wdata,
	input [15:2] dma_io_radr,
	input dma_io_radr_en,
	input [31:0] dma_io_rdata_in,
	output [31:0] dma_io_rdata

	);

// making 1 shot from level
reg int0_1lat;
reg int0_2lat;
reg int0_3lat;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n) begin
		int0_1lat <= 1'b0;
		int0_2lat <= 1'b0;
		int0_3lat <= 1'b0;
    end
	else begin
		int0_1lat <= interrupt_0;
		int0_2lat <= int0_1lat;
		int0_3lat <= int0_2lat;
	end
end

wire ext_interrupt_0_1shot = int0_2lat & ~int0_3lat;


// temporary for interrupt clear
`define SYS_INT_ENABLE 14'h3E80
`define SYS_INT_STATUS 14'h3E81

wire we_int_enable = dma_io_we      & (dma_io_wadr == `SYS_INT_ENABLE);
wire re_int_enable = dma_io_radr_en & (dma_io_radr == `SYS_INT_ENABLE);

wire we_int_status = dma_io_we      & (dma_io_wadr == `SYS_INT_STATUS);
wire re_int_status = dma_io_radr_en & (dma_io_radr == `SYS_INT_STATUS);

reg [1:0] int_enable;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		int_enable <= 2'b00;
	else if ( we_int_enable )
		int_enable <= dma_io_wdata[1:0];
end

wire int_enable_rx = int_enable[0];
wire int_enable_int0 = int_enable[1];

wire interrupt_clear_rx;

reg int_status_rx;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		int_status_rx <= 1'b0;
	else if (interrupt_clear_rx)
		int_status_rx <= 1'b0;
	else if (csr_meie & int_enable_rx & ext_uart_interrpt_1shot)
		int_status_rx <= 1'b1;
end

wire interrupt_clear_int0;

reg int_status_int0;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		int_status_int0 <= 1'b0;
	else if (interrupt_clear_int0)
		int_status_int0 <= 1'b0;
	else if (csr_meie & int_enable_int0 & ext_interrupt_0_1shot)
		int_status_int0 <= 1'b1;
end

assign g_interrupt = (int_status_rx | int_status_int0) & csr_rmie;

reg g_interrupt_dly;
always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		g_interrupt_dly <= 1'b0;
	else
		g_interrupt_dly <= g_interrupt;
end

assign g_interrupt_1shot = g_interrupt & ~g_interrupt_dly;

// clear when writing 1'b0 on the status bit

assign interrupt_clear_rx   = we_int_status & ~dma_io_wdata[0];
assign interrupt_clear_int0 = we_int_status & ~dma_io_wdata[1];


// read part
reg [1:0] re_int_dly;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		re_int_dly <= 2'd0 ;
	else
		re_int_dly <= { re_int_status, re_int_enable } ;
end

assign dma_io_rdata = (re_int_dly[0]) ? { 30'd0, int_enable } :
                      (re_int_dly[1]) ? { 30'd0, int_status_int0, int_status_rx } : dma_io_rdata_in;

endmodule
