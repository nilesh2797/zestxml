#!/usr/bin/perl

use strict;
use warnings;

my $fname = $ARGV[0];
my $fin; open( $fin, "<", $fname );
my $fout; open( $fout, ">", "tmp.txt" );

my $ctr = 0;
while( <$fin> )
{
	if( $ctr>0 )
	{
		$_ =~ s/\.0*(\s)/$1/g;		
		$_ =~ s/(\.[^ ]*?)(0*)(\s)/$1$2/g;		
	}
	print $fout "$_";
	$ctr++;
}

close( $fin );
close( $fout );

system( "mv tmp.txt $fname" );
