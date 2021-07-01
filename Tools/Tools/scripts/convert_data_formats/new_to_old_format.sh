#!/bin/bash

paste $1 $2 | perl -w -ne 'BEGIN{
	$old_f; open($old_f,">",$ARGV[0]);
	$line = 0;
	@ARGV = ();
}
	$line++,next if($line==0);

	chomp($_);
	my @items = split("\t",$_);

	my @lbls = split(" ",$items[0]);
	@lbls = map {my @idval = split(":",$_); $idval[0]} @lbls;
	print $old_f join(",",@lbls)." ".$items[1]."\n";

	$line++;
END{
	close($old_f);
}
' $3 

