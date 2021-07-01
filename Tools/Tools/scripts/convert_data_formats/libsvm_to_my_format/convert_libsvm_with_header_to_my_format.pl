#!/usr/bin/env perl

use strict;
use warnings;

sub write_fil( $$$$ )
{
	my $in_file = shift;
	my $ft_file = shift;
	my $lbl_file = shift;
	my $start_index = shift;

	my $fin; open( $fin, "<", $in_file );
	my $fft; open( $fft, ">", $ft_file );
	my $flbl; open( $flbl, ">", $lbl_file );

	my $dims_str = <$fin>;
	chomp( $dims_str );
	my @dims = split( " ", $dims_str );
	my $num_inst = $dims[0];
	my $num_ft = $dims[1];
	my $num_lbl = $dims[2];

	print $fft "$num_inst $num_ft\n";
	print $flbl "$num_inst $num_lbl\n";

	while( <$fin> )
	{
		chomp( $_ );
		my @items = split( " ", $_, 2 );

		if( $_ !~ m/^ .*$/ )
		{
			my @lbls = split( ",", $items[0] );
			@lbls = sort {$a <=> $b} @lbls;
			@lbls = map {my $l = $_; $l -= $start_index; "$l:1"} @lbls;
			print $flbl join( " ", @lbls )."\n";		
		}
		else
		{
			print $flbl "\n";
		}

		my @fts = split( " ", $items[1] );
		@fts = map {my @pair = split( ":", $_ ); $pair[0] -= $start_index; \@pair } @fts;
		@fts = sort {@$a[0] <=> @$b[0]} @fts;
		@fts = map {my @p = @$_; "$p[0]:$p[1]"} @fts;
		print $fft join( " ", @fts )."\n";
	}

	close( $fin );
	close( $fft );
	close( $flbl );
}

my $in_file = $ARGV[0];
my $ft_file = $ARGV[1];
my $lbl_file = $ARGV[2];
my $start_index = $ARGV[3];
write_fil( $in_file, $ft_file, $lbl_file, $start_index );