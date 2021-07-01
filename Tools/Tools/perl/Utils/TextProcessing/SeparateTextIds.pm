#!/usr/bin/env perl
package Utils::TextProcessing::SeparetTextIds;
use Exporter 'import';
our @EXPORT = qw(separate_text_ids_fhandles separate_text_ids_files); 

use strict;
use warnings;
use Data::Dumper;
use Text::Trim;
use feature "say";
use Getopt::Long;
use Utils::GeneralUse;

sub separate_text_ids_fhandles
{
	my $fin = shift();
	my $fout = shift();
	my $params = shift();
	my $separator = fetch_param( $params, "separator", "\t" );
	my $print_ids = fetch_param( $params, "print_ids", 0 );
	my $id_file = fetch_param( $params, "id_file", "" );
	my $print_progress = fetch_param( $params, "print_progress", 0 );
	print_subr_begin() if( $print_progress );

	#print STDERR "$separator\n";

	my $fid;
	$fid = safe_open( ">", $id_file ) if( $print_ids );
	
	my $linect=0;
	while( <$fin> )
	{
		$linect = print_loop_progress( $linect, 10000 ) if( $print_progress );
		chomp( $_ );
		my ($id,$text) = split( $separator, $_ );
		print $fid "$id\n" if( $print_ids );
		print $fout "$text\n";
	}

 	close( $fid ) if( $print_ids );
	print_subr_end() if( $print_progress );
}

sub separate_text_ids_files
{
	my $in_file = shift();
	my $out_file = shift();
	my $params = shift();
	my $fin = safe_open( "<", $in_file );
	my $fout = safe_open( ">", $out_file );
	separate_text_ids_fhandles( $fin, $fout, $params );
	close( $fin );
	close( $fout );
}

unless( caller )
{
	my %params;
	GetOptions( \%params, 'separator:s','print_ids:i','id_file:s','print_progress:i' );
	@ARGV = ();
	separate_text_ids_fhandles( *STDIN, *STDOUT, \%params );
}