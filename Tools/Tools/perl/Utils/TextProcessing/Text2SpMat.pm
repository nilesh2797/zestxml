#!/usr/bin/env perl
# usage from shell: cat [cleaned file name] | Text2SpMat.pm [splitter] [words file name] [0/1, 0=>training data, 1=>test data]  > [sparse mat file name]
#        [splitter] is usually " ", but can be anything

package Utils::TextProcessing::Text2SpMat;
use Exporter 'import';
our @EXPORT_OK = qw(text_2_spmat_fhandles text_2_spmat_files); 

use strict;
use warnings;
use File::Temp qw(tempfile);
use Getopt::Long;
use Utils::GeneralUse;
use feature "say";

sub text_2_spmat_fhandles
{
	my $fin = shift();
	my $fout = shift();
	my $words_file = shift();
	my $params = shift();
	my $separator = fetch_param( $params, "separator", " " );
	my $train_or_test = fetch_param( $params, "train_or_test", "train" );
	my $print_progress = fetch_param( $params, "print_progress", 0 );
	print_subr_begin() if $print_progress;

	my $fwords;
	if( $train_or_test eq "train" )
	{
		$fwords = safe_open( ">", $words_file );
	}
	elsif( $train_or_test eq "test" )
	{
		$fwords = safe_open( "<", $words_file );
	}
	else
	{
		die "ERROR: In Text2SpMat, invalid option $train_or_test\n";
	}

	my %words = ();
	my $linect = 0;
	my $count;
	my ($ftmp,$tmpname) = tempfile();
	while( <$fin> )
	{
		$linect = print_loop_progress( $linect, 10000 ) if $print_progress;
		$count++;
		print $ftmp "$_";
		chomp($_);
		$words{$_}++ for(split($separator,$_));	 # assumes that words are separated by single space
	}
	close($ftmp);

	my @sortwords;
	if( $train_or_test eq "train" ) #training
	{
		@sortwords = sort {$a cmp $b} keys(%words);
		say $fwords "$_" for(@sortwords);
	}
	elsif( $train_or_test eq "test" ) #test
	{
		while( <$fwords> )
		{
			chomp( $_ );
			push( @sortwords, $_ );
		}
	}

	{
		%words = ();
		my $ctr = 0; 
		$words{$_} = $ctr++ for( @sortwords );
	}

	say "$count ".scalar(keys(%words));

	$linect = 0;
	open($ftmp, "<", $tmpname) or die "ERROR: Couldn't open file, '$tmpname': $!";
	while(<$ftmp>)
	{
		$linect = print_loop_progress( $linect, 10000 ) if $print_progress;
		chomp($_);
		my %wcounts = ();
		$wcounts{$_}++ for(split($separator,$_));
		say $fout join(" ", map {"$words{$_}:$wcounts{$_}"} (sort {$a cmp $b} ( grep { exists( $words{$_} ) } keys(%wcounts) ) )  );
	}
	close($ftmp);

	close( $fwords );
	print_subr_end() if $print_progress;
}

sub text_2_spmat_files
{
	my $infile = shift();
	my $outfile = shift();
	my $wordsfile = shift();
	my $params = shift();

	my $fin = safe_open( "<", $infile );
	my $fout = safe_open( ">", $outfile );
	text_2_spmat_fhandles( $fin, $fout, $wordsfile, $params );
	close( $fin );
	close( $fout );
}

unless( caller )
{
	my $wordsfile = $ARGV[0];
	my %params;
	GetOptions( \%params, 'separator:s','train_or_test:s','print_progress:i' );
	@ARGV = ();
	text_2_spmat_fhandles( *STDIN, *STDOUT, $wordsfile, \%params );
}