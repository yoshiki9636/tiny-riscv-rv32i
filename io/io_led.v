/*
 * My RISC-V RV32I CPU
 *   FPGA LED output and GPIO Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2025 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module io_led(
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
	output [2:0] rgb_led,
	// for reading input
	input [1:0] init_uart,
	input [1:0] init_latency,
	input init_cpu_start,
	input gpi_in,
	// gpios
	inout [3:0] gpio
	);


`define SYS_LED_IO   14'h3F80
`define SYS_GPI_IN   14'h3F81
`define SYS_GPIO_OUT 14'h3F84
`define SYS_GPIO_IN  14'h3F85
`define SYS_GPIO_EN  14'h3F86

wire we_led_value = dma_io_we      & (dma_io_wadr == `SYS_LED_IO);
wire re_led_value = dma_io_radr_en & (dma_io_radr == `SYS_LED_IO);

wire re_gpi_value = dma_io_radr_en & (dma_io_radr == `SYS_GPI_IN);

wire we_gpio_out_value = dma_io_we      & (dma_io_wadr == `SYS_GPIO_OUT);
wire re_gpio_out_value = dma_io_radr_en & (dma_io_radr == `SYS_GPIO_OUT);

wire re_gpio_in_value = dma_io_radr_en & (dma_io_radr == `SYS_GPIO_IN);

wire we_gpio_en_value = dma_io_we      & (dma_io_wadr == `SYS_GPIO_EN);
wire re_gpio_en_value = dma_io_radr_en & (dma_io_radr == `SYS_GPIO_EN);

reg [2:0] led_value;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        led_value <= 3'd0 ;
	else if ( we_led_value )
		led_value <= dma_io_wdata[2:0];
end

reg re_led_value_dly;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        re_led_value_dly <= 1'b0 ;
	else
        re_led_value_dly <= re_led_value ;
end

assign rgb_led = led_value[2:0];

// gpio part


reg [3:0] gpio_out_value;
reg [3:0] gpio_en_value;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        gpio_out_value <= 4'd0 ;
	else if ( we_gpio_out_value )
		gpio_out_value <= dma_io_wdata[3:0];
end

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        gpio_en_value <= 4'd0 ;
	else if ( we_gpio_en_value )
		gpio_en_value <= dma_io_wdata[3:0];
end

reg [3:0] re_gpio_value_dly;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        re_gpio_value_dly <= 4'd0 ;
	else
        re_gpio_value_dly <= { re_gpio_en_value, re_gpio_in_value, re_gpio_out_value, re_gpi_value };
end

wire [3:0] gpio_in;
reg [5:0] gpi_init_lat2;
reg [3:0] gpio_in_lat2;

assign dma_io_rdata = (re_led_value_dly == 1'b1) ? { 29'd0, led_value[2:0] } : 
                      (re_gpio_value_dly[0] == 1'b1) ? { 26'd0, gpi_init_lat2 } :
                      (re_gpio_value_dly[1] == 1'b1) ? { 26'd0, gpio_out_value } :
                      (re_gpio_value_dly[2] == 1'b1) ? { 26'd0, gpio_in_lat2 } :
                      (re_gpio_value_dly[3] == 1'b1) ? { 26'd0, gpio_en_value } : dma_io_rdata_in;

// inout port
assign gpio[0] = (gpio_en_value[0]) ? gpio_out_value[0] : 1'bz;
assign gpio[1] = (gpio_en_value[1]) ? gpio_out_value[1] : 1'bz;
assign gpio[2] = (gpio_en_value[2]) ? gpio_out_value[2] : 1'bz;
assign gpio[3] = (gpio_en_value[3]) ? gpio_out_value[3] : 1'bz;

assign gpio_in = gpio; 

// double latch for input signals
reg [5:0] gpi_init_lat1;
reg [3:0] gpio_in_lat1;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n) begin
        gpi_init_lat1 <= 6'd0 ;
        gpi_init_lat2 <= 6'd0 ;
        gpio_in_lat1 <= 4'd0 ;
        gpio_in_lat2 <= 4'd0 ;
	end
	else begin
        gpi_init_lat1 <= { init_uart[1:0], init_cpu_start, init_latency[1:0], gpi_in };
        gpi_init_lat2 <= gpi_init_lat1;
        gpio_in_lat1 <= gpio_in;
        gpio_in_lat2 <= gpio_in_lat1;
	end
end



endmodule
