#!/bin/perl

open (F0, ">test_dummy.txt") or die "cannot open test_dummy.txt";

$phase = 0;
$buf = "";

while (<>) {
	if (/^\s*(........)$/) {
		$buf = $1.$buf;
		$phase++;
		if ($phase >= 4) {
			print F0 "$buf\n";
			$buf = "";
			$phase = 0;
		}
	}
}
if ($phase == 1) {
	$buf = "00000000".$buf;
	print F0 "$buf\n";
}
elsif ($phase == 2) {
	$buf = "0000000000000000".$buf;
	print F0 "$buf\n";
}
elsif ($phase == 3) {
	$buf = "000000000000000000000000".$buf;
	print F0 "$buf\n";
}

close (F0);
