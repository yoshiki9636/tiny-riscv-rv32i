#!/usr/bin/perl

$flg = 0;

while(<>) {
	if(/tt_um_yoshiki/) {
		$flg = 1;
		print;
	}
	elsif ($flg == 0)  {
		print;
	}
}

