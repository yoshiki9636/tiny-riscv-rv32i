#!/usr/bin/perl

$flg = 0;

while(<>) {
	if(/tt_um_yoshiki/) {
		$flg = 1;
	}
	elsif ((/polygon id/)&&($flg == 1))  {
		print;
	}
}

