#!/usr/bin/perl
#/*
# * My RISC-V RV32I CPU
# *   Code Changer from Text Dump to ASCII code for UART Sending
# *    Perl code
# * @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
# * @copylight	2021 Yoshiki Kurokawa
# * @license		https://opensource.org/licenses/MIT     MIT license
# * @version		0.1
# */

$cntr = 0;

for ($i = 0; $i < 0x100; $i++) {
	printf("%02x", $i);
	if ($cntr == 3) {
		print "\n";
		$cntr = 0;
	}
	else {
		$cntr++;
	}
}

$cntr = 0;

for ($i = 0xff; $i >= 0; $i--) {
	printf("%02x", $i);
	if ($cntr == 3) {
		print "\n";
		$cntr = 0;
	}
	else {
		$cntr++;
	}
}

