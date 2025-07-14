#!/bin/perl

open (F, ">mem_init.txt") or die "cannot open mem_init.txt";

$phase = 0;
$address = 0;

while (<>) {
	if (/^\s*(........)$/) {
		if ($phase == 0) {
			$prev = $1;
		}
		else {
			printf F "%08x MEMO %s%s\n",$address,$1,$prev;
		}
		$phase++;
		if ($phase >= 2) { $phase = 0; }
	}
}

if ($phase == 1) {
	printf F "%08x MEMO 00000000%s\n",$address,$prev;
}

close (F);
