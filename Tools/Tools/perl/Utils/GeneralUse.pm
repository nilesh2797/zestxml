#!/usr/bin/perl
package Utils::GeneralUse;
use strict;
use warnings;
use Exporter 'import';
our @EXPORT = qw(fetch_param safe_open print_loop_progress print_subr_begin print_subr_end); 

sub fetch_param
{
	my $inparams = shift();
	my %params = %{ $inparams };
	my $pname = shift();
	my $default = shift();
	my $retval = exists( $params{ $pname } ) ? $params{ $pname } : $default;
	return $retval;
}

sub safe_open
{
	my $mode = shift();
	my $file_name = shift();
	my $f; open( $f, $mode, $file_name ) or die "ERROR: Couldn't open file, '$file_name': $!";
	return $f;
}

sub print_loop_progress
{
	my $linect = shift();
	my $window = shift();
	print STDERR "\r===loop: $linect===\t\t" if( $linect % $window == 0 );
	$linect++;
	return $linect;
}

sub print_subr_begin
{
	my $parent = ( caller(1) )[3];
	print STDERR "Begin $parent\n";
}

sub print_subr_end
{
	my $parent = ( caller(1) )[3];
	print STDERR "End $parent\n";
}

1;