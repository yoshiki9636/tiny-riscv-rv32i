/*
 * Copyright (c) 2024 Your Name
 * SPDX-License-Identifier: Apache-2.0
 */

`default_nettype none

module tt_um_yoshiki9636_tiny_riscv_rv32i_test1 (
    input  wire [7:0] ui_in,    // Dedicated inputs
    output wire [7:0] uo_out,   // Dedicated outputs
    input  wire [7:0] uio_in,   // IOs: Input path
    output wire [7:0] uio_out,  // IOs: Output path
    output wire [7:0] uio_oe,   // IOs: Enable path (active high: 0=input, 1=output)
    input  wire       ena,      // always 1 when the design is powered, so you can ignore it
    input  wire       clk,      // clock
    input  wire       rst_n     // reset_n - low to reset
);

wire [2:0] rgb_led;
wire tx;
wire rx = ui_in[0];
wire interrupt_0 = ui_in[1];
wire [3:0] sio_i = uio_in[3:0];
wire [3:0] sio_o;
wire sio_en;
wire sck;
wire [2:0] ce_n;
wire [3:0] gpio_o;
wire [3:0] gpio_i = uio_in[7:4];
wire [3:0] gpio_en;
wire init_qspicmd = ui_in[2];
wire [1:0] init_latency = ui_in[4:3];
wire init_cpu_start = ui_in[5];
wire [1:0] init_uart = ui_in[7:6];

  // All output pins must be assigned. If not used, assign to 0.
  assign uo_out  =  { rgb_led, sck, ce_n, tx };
  assign uio_out = { gpio_o, sio_o };
  assign uio_oe  = { gpio_en, { 4{ sio_en }} };

fpga_top fpga_top (
	.clkin(clk),
	.rst_n(rst_n),
	.interrupt_0(interrupt_0),
	.sck(sck),
	.ce_n(ce_n),
	.sio_i(sio_i),
	.sio_o(sio_o),
	.sio_en(sio_en),
	.rx(rx),
	.tx(tx),
	.rgb_led(rgb_led),
	.gpio_i(gpio_i),
	.gpio_o(gpio_o),
	.gpio_en(gpio_en),
	.init_qspicmd(init_qspicmd),
	.init_latency(init_latency),
	.init_cpu_start(init_cpu_start),
	.init_uart(init_uart)
	);


  // List all unused inputs to prevent warnings
  wire _unused = &{ena, clk, rst_n, 1'b0};

endmodule

