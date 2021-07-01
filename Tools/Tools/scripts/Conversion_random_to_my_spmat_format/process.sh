#!/bin/bash

cat scores.0.txt | perl -w -ne 'BEGIN{$ctr=0;}
$ctr++;
chomp($_);
@items = split(" ",$_);

for $item(@items)
{
	@idval = split(":",$item);
	print "$ctr $idval[0] $idval[1]\n"
}

print STDERR "$ctr\n" if($ctr%10000==0);

' > triads.0.txt
