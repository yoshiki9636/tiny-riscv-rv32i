/*
 * My RISC-V RV32I CPU
 *   UART Monitor Record Character and Decode Command Module
 *    Verilog code
 * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
 * @copylight	2021 Yoshiki Kurokawa
 * @license		https://opensource.org/licenses/MIT     MIT license
 * @version		0.1
 */

module uart_rec_char (
    input clk,
    input rst_n,
	input [7:0] rout,
	input rout_en,
	
	// from ctrl logics
	input dump_running,
	input trush_running,
	//input dcflush_running,
	// to ctrl logics
	output [31:0] uart_data,
	output reg cpu_start,
	input cpu_run_state,
	output write_address_set,
	output write_data_en,
	output read_start_set,
	output read_end_set,
	output read_stop,
	output start_trush,
	output start_step,
	output quit_cmd,
	output pgm_start_set,
	output pgm_end_set,
	output pgm_stop,
	output inst_address_set,
	output inst_data_en,
	output pc_print,
	output pc_print_sel,
	output crlf_in
	//output reg [3:0] cmd_status,
	//output reg data_en,
	//output [2:0] test
	
);

// read data decoder

reg [7:0] pdata;
reg data_en;

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
        pdata <= 8'd0;
    else if (rout_en)
        pdata <= rout;
end

always @ (posedge clk or negedge rst_n) begin
    if (~rst_n)
		data_en <= 1'b0;
	else
		data_en <= rout_en;
end

function [26:0] data_decoder;
input [7:0] pdata;
begin
	case(pdata)
		8'h30 : data_decoder = 27'b000_0000_0000_0000_0000_0000_0001; // 0
		8'h31 : data_decoder = 27'b000_0000_0000_0000_0000_0000_0010; // 1
		8'h32 : data_decoder = 27'b000_0000_0000_0000_0000_0000_0100; // 2
		8'h33 : data_decoder = 27'b000_0000_0000_0000_0000_0000_1000; // 3
		8'h34 : data_decoder = 27'b000_0000_0000_0000_0000_0001_0000; // 4
		8'h35 : data_decoder = 27'b000_0000_0000_0000_0000_0010_0000; // 5
		8'h36 : data_decoder = 27'b000_0000_0000_0000_0000_0100_0000; // 6
		8'h37 : data_decoder = 27'b000_0000_0000_0000_0000_1000_0000; // 7
		8'h38 : data_decoder = 27'b000_0000_0000_0000_0001_0000_0000; // 8
		8'h39 : data_decoder = 27'b000_0000_0000_0000_0010_0000_0000; // 9
		8'h61 : data_decoder = 27'b000_0000_0000_0000_0100_0000_0000; // a
		8'h62 : data_decoder = 27'b000_0000_0000_0000_1000_0000_0000; // b
		8'h63 : data_decoder = 27'b000_0000_0000_0001_0000_0000_0000; // c
		8'h64 : data_decoder = 27'b000_0000_0000_0010_0000_0000_0000; // d
		8'h65 : data_decoder = 27'b000_0000_0000_0100_0000_0000_0000; // e
		8'h66 : data_decoder = 27'b000_0000_0000_1000_0000_0000_0000; // f
		8'h67 : data_decoder = 27'b000_0000_0001_0000_0000_0000_0000; // g : go PC to address (run program)
		8'h03 : data_decoder = 27'b000_0000_0010_0000_0000_0000_0000; // Ctri-c : quit,stop,finish
		8'h77 : data_decoder = 27'b000_0000_0100_0000_0000_0000_0000; // w : write data memory
		8'h72 : data_decoder = 27'b000_0000_1000_0000_0000_0000_0000; // r : read data memory and dump
		8'h74 : data_decoder = 27'b000_0001_0000_0000_0000_0000_0000; // t : trushed memory and 0 clear
		8'h73 : data_decoder = 27'b000_0010_0000_0000_0000_0000_0000; // s : step execution
		8'h70 : data_decoder = 27'b000_0100_0000_0000_0000_0000_0000; // p : read IO
		8'h69 : data_decoder = 27'b000_1000_0000_0000_0000_0000_0000; // i : write IO
		8'h6a : data_decoder = 27'b001_0000_0000_0000_0000_0000_0000; // j : print PC
		8'h7a : data_decoder = 27'b010_0000_0000_0000_0000_0000_0000; // z : flush all D-cache
		8'h0d : data_decoder = 27'b100_0000_0000_0000_0000_0000_0000; // CR : change to CRLF
		default : data_decoder = 27'd0;
	endcase
end
endfunction

wire [26:0] decode_bits = data_decoder( pdata );

function [3:0] bin_encoder;
input [15:0] hot_code;
begin
	case(hot_code)
		16'b0000_0000_0000_0001 : bin_encoder = 4'h0;
		16'b0000_0000_0000_0010 : bin_encoder = 4'h1;
		16'b0000_0000_0000_0100 : bin_encoder = 4'h2;
		16'b0000_0000_0000_1000 : bin_encoder = 4'h3;
		16'b0000_0000_0001_0000 : bin_encoder = 4'h4;
		16'b0000_0000_0010_0000 : bin_encoder = 4'h5;
		16'b0000_0000_0100_0000 : bin_encoder = 4'h6;
		16'b0000_0000_1000_0000 : bin_encoder = 4'h7;
		16'b0000_0001_0000_0000 : bin_encoder = 4'h8;
		16'b0000_0010_0000_0000 : bin_encoder = 4'h9;
		16'b0000_0100_0000_0000 : bin_encoder = 4'ha;
		16'b0000_1000_0000_0000 : bin_encoder = 4'hb;
		16'b0001_0000_0000_0000 : bin_encoder = 4'hc;
		16'b0010_0000_0000_0000 : bin_encoder = 4'hd;
		16'b0100_0000_0000_0000 : bin_encoder = 4'he;
		16'b1000_0000_0000_0000 : bin_encoder = 4'hf;
		default : bin_encoder = 4'h0;
	endcase
end
endfunction

wire [3:0] half_data = bin_encoder( decode_bits[15:0] ) ;
wire num_char = | decode_bits[15:0];
//assign test = decode_bits[18:16];
//assign test = pdata[2:0];

wire cmd_g = decode_bits[16] & data_en;
wire cmd_q = decode_bits[17] & data_en;
wire cmd_w = decode_bits[18] & data_en;
wire cmd_r = decode_bits[19] & data_en;
wire cmd_t = decode_bits[20] & data_en;
wire cmd_s = decode_bits[21] & data_en;
wire cmd_p = decode_bits[22] & data_en;
wire cmd_i = decode_bits[23] & data_en;
wire cmd_j = decode_bits[24] & data_en;
wire cmd_z = decode_bits[25] & data_en;
wire cmd_crlf = decode_bits[26] & data_en;

// command format
// g : goto PC address ( run program until quit ) : format:  g <start addess>
// Ctrl-c : quit from any command                 : format: Ctrl-c
// w : write date to memory                       : format:  w <start adderss> <data> ....<data> q
// r : read data from memory                      : format:  r <start address> <end adderss>
// t : trashed memory data and 0 clear            : format:  t
// s : program step execution                     : format:  s
// p : read instruction memory                    : format:  p <start address> <end adderss>
// i : write instruction memory                   : format:  i <start adderss> <data> ....<data> q
// j : print current PC value                     : format:  j
// z : flush D-cache                              : format:  z
// state machine

reg word_valid;

reg [4:0] cmd_status;
wire [4:0] next_cmd_status;

`define C_STAIDLE 5'd0
`define C_GSETNUM 5'd1
`define C_GOTONUM 5'd2
`define C_WADRNUM 5'd3
`define C_WDATNUM 5'd4
`define C_RSTARTN 5'd5
`define C_RENDNUM 5'd6
`define C_RDUMPDT 5'd7
`define C_TRUSHDT 5'd8
`define C_STEPRUN 5'd9 // not implimented
`define C_IRDSTAR 5'd10
`define C_IRDENDN 5'd11
`define C_IRDDUMP 5'd12
`define C_IWTADRN 5'd13
`define C_IWTDATA 5'd14
`define C_PCPRINT 5'd15
`define C_DCFLUSH 5'd16


function [4:0] cmd_statemachine;
input [4:0] cmd_status;
input cmd_g;
input cmd_q;
input cmd_w;
input cmd_r;
input cmd_t;
input cmd_s;
input cmd_p;
input cmd_i;
input cmd_j;
input cmd_z;
input word_valid;
input dump_running;
input trush_running;
//input dcflush_running;

begin
	case(cmd_status)
		`C_STAIDLE :
			casez({cmd_j,cmd_g,cmd_w,cmd_r,cmd_t,cmd_s,cmd_p,cmd_i,cmd_z})
				9'b1_????_???? : cmd_statemachine = `C_PCPRINT;
				9'b0_1???_???? : cmd_statemachine = `C_GSETNUM;
				9'b0_01??_???? : cmd_statemachine = `C_WADRNUM;
				9'b0_001?_???? : cmd_statemachine = `C_RSTARTN;
				9'b0_0001_???? : cmd_statemachine = `C_TRUSHDT;
				9'b0_0000_1??? : cmd_statemachine = `C_STEPRUN;
				9'b0_0000_01?? : cmd_statemachine = `C_IRDSTAR;
				9'b0_0000_001? : cmd_statemachine = `C_IWTADRN;
				9'b0_0000_0001 : cmd_statemachine = `C_DCFLUSH;
				default   : cmd_statemachine = `C_STAIDLE;
			endcase
		`C_GSETNUM :
			casez({cmd_q,word_valid})
				2'b1? : cmd_statemachine = `C_STAIDLE;
				2'b01 : cmd_statemachine = `C_GOTONUM;
				default : cmd_statemachine = `C_GSETNUM;
			endcase
		`C_GOTONUM :
			if (cmd_q)
				cmd_statemachine = `C_STAIDLE;
			else
				cmd_statemachine = `C_GOTONUM;
		`C_WADRNUM :
			casez({cmd_q,word_valid})
				2'b1? : cmd_statemachine = `C_STAIDLE;
				2'b01 : cmd_statemachine = `C_WDATNUM;
				default : cmd_statemachine = `C_WADRNUM;
			endcase
		`C_WDATNUM :
			if (cmd_q)
				cmd_statemachine = `C_STAIDLE;
			else
				cmd_statemachine = `C_WDATNUM;
		`C_RSTARTN :
			casez({cmd_q,word_valid})
				2'b1? : cmd_statemachine = `C_STAIDLE;
				2'b01 : cmd_statemachine = `C_RENDNUM;
				default : cmd_statemachine = `C_RSTARTN;
			endcase
		`C_RENDNUM :
			casez({cmd_q,word_valid})
				2'b1? : cmd_statemachine = `C_STAIDLE;
				2'b01 : cmd_statemachine = `C_RDUMPDT;
				default : cmd_statemachine = `C_RENDNUM;
			endcase
		`C_RDUMPDT :
			if (cmd_q | ~dump_running)
				cmd_statemachine = `C_STAIDLE;
			else
				cmd_statemachine = `C_RDUMPDT;
		`C_TRUSHDT :
			if (cmd_q | ~trush_running)
				cmd_statemachine = `C_STAIDLE;
			else
				cmd_statemachine = `C_TRUSHDT;
		`C_STEPRUN :
			cmd_statemachine = `C_STAIDLE;
		`C_IRDSTAR :
			casez({cmd_q,word_valid})
				2'b1? : cmd_statemachine = `C_STAIDLE;
				2'b01 : cmd_statemachine = `C_IRDENDN;
				default : cmd_statemachine = `C_IRDSTAR;
			endcase
		`C_IRDENDN :
			casez({cmd_q,word_valid})
				2'b1? : cmd_statemachine = `C_STAIDLE;
				2'b01 : cmd_statemachine = `C_IRDDUMP;
				default : cmd_statemachine = `C_IRDENDN;
			endcase
		`C_IRDDUMP :
			if (cmd_q | ~dump_running)
				cmd_statemachine = `C_STAIDLE;
			else
				cmd_statemachine = `C_IRDDUMP;
		`C_IWTADRN :
			casez({cmd_q,word_valid})
				2'b1? : cmd_statemachine = `C_STAIDLE;
				2'b01 : cmd_statemachine = `C_IWTDATA;
				default : cmd_statemachine = `C_IWTADRN;
			endcase
		`C_IWTDATA :
			if (cmd_q)
				cmd_statemachine = `C_STAIDLE;
			else
				cmd_statemachine = `C_IWTDATA;
		`C_PCPRINT :
			if (cmd_q | ~dump_running)
				cmd_statemachine = `C_STAIDLE;
			else
				cmd_statemachine = `C_PCPRINT;
		`C_DCFLUSH :
			if (cmd_q )
				cmd_statemachine = `C_STAIDLE;
			else
				cmd_statemachine = `C_DCFLUSH;
		default : cmd_statemachine = `C_STAIDLE;
	endcase
end
endfunction

assign next_cmd_status = cmd_statemachine(
							cmd_status,
							cmd_g,
							cmd_q,
							cmd_w,
							cmd_r,
							cmd_t,
							cmd_s,
							cmd_p,
							cmd_i,
							cmd_j,
							cmd_z,
							word_valid,
							dump_running,
							trush_running
							);

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		cmd_status <= `C_STAIDLE;
	else
		cmd_status <= next_cmd_status;
end

// decode signals

wire idle_status = ( cmd_status == `C_STAIDLE );
wire gcmd_setnum = ( cmd_status == `C_GSETNUM );
wire gcmd_setrun = ( cmd_status == `C_GOTONUM );
wire wcmd_setnum = ( cmd_status == `C_WADRNUM );
wire wcmd_setdat = ( cmd_status == `C_WDATNUM );
wire rcmd_setsta = ( cmd_status == `C_RSTARTN );
wire rcmd_setend = ( cmd_status == `C_RENDNUM );
wire rcmd_dumpdt = ( cmd_status == `C_RDUMPDT );
wire pcmd_setsta = ( cmd_status == `C_IRDSTAR );
wire pcmd_setend = ( cmd_status == `C_IRDENDN );
wire pcmd_dumpdt = ( cmd_status == `C_IRDDUMP );
wire icmd_setnum = ( cmd_status == `C_IWTADRN );
wire icmd_setdat = ( cmd_status == `C_IWTDATA );
wire jcmd_pcprnt = ( cmd_status == `C_PCPRINT );

wire g_crlf = ( cmd_status == `C_GSETNUM ) & ( next_cmd_status == `C_GOTONUM );
wire r_crlf = ( cmd_status == `C_RENDNUM ) & ( next_cmd_status == `C_RDUMPDT );
wire w_crlf = ( cmd_status == `C_WADRNUM ) & ( next_cmd_status == `C_WDATNUM );
wire p_crlf = ( cmd_status == `C_IRDENDN ) & ( next_cmd_status == `C_IRDDUMP );
wire i_crlf = ( cmd_status == `C_IWTADRN ) & ( next_cmd_status == `C_IWTDATA );
assign pc_print_sel = (cmd_status == `C_PCPRINT);
assign pc_print = idle_status & (next_cmd_status == `C_PCPRINT);

wire cmd_t_crlf = cmd_t & ~cpu_run_state;
wire cmd_s_crlf = cmd_s & ~cpu_run_state;
wire cmd_j_crlf = cmd_j & ~cpu_run_state;
wire cmd_z_crlf = cmd_z & ~cpu_run_state;

assign crlf_in = g_crlf | cmd_q | r_crlf | w_crlf | cmd_t_crlf | cmd_s_crlf | p_crlf | i_crlf | cmd_crlf | cmd_j_crlf | cmd_z_crlf;

// data setter

wire bin_data_set = data_en & num_char & (gcmd_setnum | wcmd_setnum | wcmd_setdat | rcmd_setsta | rcmd_setend
                                          | pcmd_setsta | pcmd_setend | icmd_setnum | icmd_setdat);

wire word_start =  idle_status & (( next_cmd_status == `C_GSETNUM )|( next_cmd_status == `C_WADRNUM )|
								  ( next_cmd_status == `C_RSTARTN )|( next_cmd_status == `C_IRDSTAR )|
								  ( next_cmd_status == `C_IWTADRN ));

reg [31:0] data_word;
reg [31:0] data_word_out;
reg [3:0] data_cntr;
wire word_valid_pre;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		data_word <= 32'd0;
	else if (cmd_q)
		data_word <= 32'd0;
	else if (bin_data_set)
		data_word <= { data_word[27:0], half_data };
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		data_word_out <= 32'd0;
	else if (word_valid_pre)
		data_word_out <= { data_word[27:0], half_data };
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		data_cntr <= 4'd0;
	else if (word_start)
		data_cntr <= 4'd0;		
	else if (cmd_q)
		data_cntr <= 4'd0;
	else if (data_cntr >= 4'd8)
		data_cntr <= 4'd0;
	else if (bin_data_set)
		data_cntr <= data_cntr + 4'd1;
end

assign word_valid_pre = (data_cntr == 4'd7) & bin_data_set;

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		word_valid <= 1'b0;
	else
		word_valid <= word_valid_pre;
end

// g command

wire ctrl_valid = gcmd_setnum | wcmd_setnum | wcmd_setdat | rcmd_setsta | rcmd_setend
							  | pcmd_setsta | pcmd_setend | icmd_setnum | icmd_setdat;


assign uart_data = data_word_out;
/*
always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		uart_data <= 32'd0;
	else if (ctrl_valid & word_valid)
		uart_data <= data_word;
end
*/

reg g_crlf_dly;
reg g_crlf_dly2;
always @ (posedge clk or negedge rst_n) begin
	if (~rst_n) begin
		g_crlf_dly <= 1'b0;
		g_crlf_dly2 <= 1'b0;
		cpu_start <= 1'b0;
	end
	else begin
		g_crlf_dly <= g_crlf;
		g_crlf_dly2 <= g_crlf_dly;
		cpu_start <= g_crlf_dly2;
		//cpu_start <= gcmd_setrun;
	end
end

assign write_address_set = wcmd_setnum & word_valid;
assign write_data_en = wcmd_setdat & word_valid;
assign read_start_set = rcmd_setsta & word_valid;
assign read_end_set = rcmd_setend & word_valid;
assign read_stop = rcmd_dumpdt & cmd_q;
assign start_trush = idle_status & cmd_t;
assign start_step = idle_status & cmd_s;
assign quit_cmd = cmd_q;
assign pgm_start_set = pcmd_setsta & word_valid;
assign pgm_end_set = pcmd_setend & word_valid;
assign pgm_stop = pcmd_dumpdt & cmd_q;
assign inst_address_set = icmd_setnum & word_valid;
assign inst_data_en = icmd_setdat & word_valid;

/*
// w command

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		write_address_set <= 1'b0;
	else
		write_address_set <= wcmd_setnum & word_valid;
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		write_data_en <= 1'b0;
	else
		write_data_en <= wcmd_setdat & word_valid;
end

// r command

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		read_start_set <= 1'b0;
	else
		read_start_set <= rcmd_setsta & word_valid;
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		read_end_set <= 1'b0;
	else
		read_end_set <= rcmd_setend & word_valid;
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		read_stop <= 1'b0;
	else
		read_stop <= rcmd_dumpdt & cmd_q;
end

// t command

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		start_trush <= 1'b0;
	else
		start_trush <= idle_status & cmd_t;
end

// s command

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		start_step <= 1'b0;
	else
		start_step <= idle_status & cmd_s;
end

// q command

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		quit_cmd <= 1'b0;
	else
		quit_cmd <= cmd_q;
end

// p command

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		pgm_start_set <= 1'b0;
	else
		pgm_start_set <= pcmd_setsta & word_valid;
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		pgm_end_set <= 1'b0;
	else
		pgm_end_set <= pcmd_setend & word_valid;
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		pgm_stop <= 1'b0;
	else
		pgm_stop <= pcmd_dumpdt & cmd_q;
end

// i command

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		inst_address_set <= 1'b0;
	else
		inst_address_set <= icmd_setnum & word_valid;
end

always @ (posedge clk or negedge rst_n) begin
	if (~rst_n)
		inst_data_en <= 1'b0;
	else
		inst_data_en <= icmd_setdat & word_valid;
end
*/

endmodule
