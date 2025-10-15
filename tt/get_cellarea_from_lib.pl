#!/usr/bin/perl

@files = @ARGV;

$flg = 0;
@cells = ();
%num = {};
%area = {};
%sum = {};

@ARGV = ( $files[0] );

while(<>) {
	if((/Contents\s+of\s+circuit\s+1.+yoshiki/)&&($flg == 0)) {
		$flg = 1;
	}
	elsif(/Contents of circuit 2.+yoshiki/) {
		$flg = 2;
	}
	elsif ((/\s+Class:\s+sky130_fd_sc_hd__(\S+)\s+instances:\s+([0-9]+)/)&&($flg == 1))  {
		#print "$1,$2\n";
		push(@cells, $1);
		$num{$1} = $2;
	}
}


@ARGV = ( $files[1] );
$sw = 0;

while(<>) {
	if ((/\s+cell\s+\(\"sky130_fd_sc_hd__(\S+)\"\)/)&&($sw == 0)) {
		$hit = 0;
		foreach $cell (@cells) {
			if ($cell eq $1) {
				$hit = 1;
				break;
			}
		}
		if ($hit == 1) {
			$sw = 1;
			$idx = $1;
		}
		else {
			$sw = 0;
		}
	}
	elsif ((/\s+area\s+:\s+([0-9.]+);/)&&($sw == 1)) {
		$sw = 0;
		$area{$idx} = $1;
		$sum{$idx} = $area{$idx} * $num{$idx};
		print "$idx,$area{$idx},$num{$idx},$sum{$idx}\n";
	}
}

$all = 0.0;
$combi = 0.0;
$ffs = 0.0;
$clk = 0.0;
$dly = 0.0;
$decap = 0.0;

foreach $cell (@cells) {
	$all += $sum{$cell};
	if ($cell =~ /df/) {
		$ffs += $sum{$cell};
	}
	elsif ($cell =~ /clk/) {
		$clk += $sum{$cell};
	}
	elsif ($cell =~ /dly/) {
		$dly += $sum{$cell};
	}
	elsif ($cell =~ /decap/) {
		$decap += $sum{$cell};
	}
}

$combi = $all - $ffs - $clk - $dly - $decap;

print "all,$all\n";
print "combi,$combi\n";
print "ffs,$ffs\n";
print "clk,$clk\n";
print "dly,$dly\n";
print "decap,$decap\n";

