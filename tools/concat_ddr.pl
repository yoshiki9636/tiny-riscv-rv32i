#!/bin/perl

use Getopt::Long;

GetOptions('p=s' => \$p);

if (defined($p)) {
    if ($p =~ /^0x/) { $pc = hex($p); }
    else { $pc = int($p); }
}
else {
    $pc = 0;
}

$pc = $pc / 2;

open (F0, ">mem_init.txt") or die "cannot open test_dummy.txt";

$phase = 0;
$buf = "";

while (<>) {
	if (/^\s*(........)$/) {
		$buf = $1.$buf;
		$phase++;
		if ($phase >= 4) {
			if (defined($p)) {
				$col = $pc & 0x3ff;
				$bank = ($pc >> 10) & 0x7;
				$row = ($pc >> 13) & 0x3fff;
				$pc2 = ($bank << 24) + ($row << 10) + $col;
				$pcadr = sprintf("%08x",$pc2);
				print F0 "$pcadr PRG $buf\n";
				$pc += 8;
			}
			else {
				print F0 "$buf\n";
			}
			$buf = "";
			$phase = 0;
		}
	}
}
$col = $pc & 0x3ff;
$bank = ($pc >> 10) & 0x7;
$row = ($pc >> 13) & 0x3fff;
$pc2 = ($bank << 24) + ($row << 10) + $col;
#$col = $pc & 0x3ff;
#$row = ($pc >> 10) & 0x3fff;
#$bank = ($pc >> 24) & 0x7;
#$pc2 = ($row << 13) + ($bank << 10) + $col;
$pcadr = sprintf("%08x",$pc2);
if ($phase == 1) {
	$buf = "00000000".$buf;
	if (defined($p)) {
		print F0 "$pcadr PRG $buf\n";
	}
	else {
		print F0 "$buf\n";
	}
}
elsif ($phase == 2) {
	$buf = "0000000000000000".$buf;
	if (defined($p)) {
		print F0 "$pcadr PRG $buf\n";
	}
	else {
		print F0 "$buf\n";
	}
}
elsif ($phase == 3) {
	$buf = "000000000000000000000000".$buf;
	if (defined($p)) {
		print F0 "$pcadr PRG $buf\n";
	}
	else {
		print F0 "$buf\n";
	}
}

close (F0);
