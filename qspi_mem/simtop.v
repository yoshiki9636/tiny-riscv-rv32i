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

reg read_req; // input
reg read_w; // input
reg read_hw; // input
wire read_valid; // output
reg [31:0] read_adr; // input
wire [31:0] read_data; // output

reg write_req; // input
reg write_w; // input
reg write_hw; // input
wire write_finish; // output
reg [31:0] write_adr; // input
reg [31:0] write_data; // input

// test module
qspi_if qspi_if (
	.clk(clk),
	.rst_n(rst_n),
	.sck(sck),
	.ce_n(ce_n),
	.sio(sio),
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
	.write_data(write_data)
	);
// psram model
qspi_psram_model qspi_psram_model (
	.clk(clk),
	.rst_n(rst_n),
	.sck(sck),
	.ce_n(ce_n),
	.sio(sio)
	);

//initial $readmemh("./test.txt", fpga_top.cpu_top.if_stage.inst_1r1w.ram);

initial clk = 0;

always #5 clk <= ~clk;


initial begin
	rst_n = 1'b1;
	read_req = 1'b0;
	read_w = 1'b0;
	read_hw = 1'b0;
	read_adr = 32'd0;
	write_req = 1'b0;
	write_w = 1'b0;
	write_hw = 1'b0;
	write_adr = 32'd0;
	write_data = 32'd0;
#10
	rst_n = 1'b0;
#20
	rst_n = 1'b1;
#10
	write_req = 1'b1;
	write_w = 1'b1;
	write_hw = 1'b0;
	write_adr = 32'h0000_1000;
	write_data = 32'h8765_4321;
#10
	write_req = 1'b0;
#4200
	write_req = 1'b1;
	write_w = 1'b0;
	write_hw = 1'b1;
	write_adr = 32'h0000_1002;
	write_data = 32'h0000_abcd;
#10
	write_req = 1'b0;
#3400
	write_req = 1'b1;
	write_w = 1'b0;
	write_hw = 1'b0;
	write_adr = 32'h0000_1003;
	write_data = 32'h0000_00ef;
#10
	write_req = 1'b0;
#3400
	read_req = 1'b1;
	read_w = 1'b1;
	read_hw = 1'b0;
	read_adr = 32'h0000_1000;
#10
	read_req = 1'b0;
#5600
	read_req = 1'b1;
	read_w = 1'b0;
	read_hw = 1'b1;
	read_adr = 32'h0000_1002;
#10
	read_req = 1'b0;
#5600
	read_req = 1'b1;
	read_w = 1'b0;
	read_hw = 1'b0;
	read_adr = 32'h0000_1003;
#10
	read_req = 1'b0;
#5600



	$stop;
end

endmodule
