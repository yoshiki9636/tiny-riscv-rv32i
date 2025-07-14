/*
 * My RISC-V RV32I CPU
 *   Verilog Simulation Top Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2021 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module simtop;

reg clk;
reg rst_n;

wire sck;
wire ce_n;
wire [3:0] sio;

reg interrupt_0;
reg rx;
wire tx;
wire [3:0] rgb_led; 

fpga_top fpga_top (
	.clk(clk),
	.rst_n(rst_n),
	.interrupt_0(interrupt_0),
	.sck(sck),
	.ce_n(ce_n),
	.sio(sio),
	.rx(rx),
	.tx(tx),
	.rgb_led(rgb_led)
	);

// psram model
qspi_psram_model qspi_psram_model (
	.clk(clk),
	.rst_n(rst_n),
	.sck(sck),
	.ce_n(ce_n),
	.sio(sio)
	);

initial $readmemh("./test.txt", qspi_psram_model.qspi_1r1w.ram);

initial clk = 0;

always #5 clk <= ~clk;


initial begin
	rst_n = 1'b1;
	interrupt_0 = 1'b0;
	rx = 1'b0;
#10
	rst_n = 1'b0;
#20
	rst_n = 1'b1;
#100
	force fpga_top.cpu_start = 1'b1;
    //force fpga_top.cpu_start_adr = 30'h00000400; // 0x1000 for C
    //force fpga_top.cpu_start_adr = 30'h00000040; // 0x100 for asm
    force fpga_top.cpu_start_adr = 30'h00000000; // 0x0 for test
#10
	force fpga_top.cpu_start = 1'b0;
	read_req = 1'b0;
#10000
	$stop;
end

endmodule
