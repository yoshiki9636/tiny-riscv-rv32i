#!/bin/perl


while(<>) {
	if (/^M\s+([0-9abcdef]+)\s+\(([0-9abcdef]+)\)/) {
		$addr = hex($1);
		$inst = hex($2);
		printf("instlig: %08x , %08x\n",$addr,$inst);
	}
}

