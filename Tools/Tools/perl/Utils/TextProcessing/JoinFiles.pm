#!/usr/bin/env perl
# Join multiple files line by line

package Utils::TextProcessing::JoinFiles;
use Exporter 'import';
our @EXPORT = qw(join_files);

use strict;
use warnings;
use Data::Dumper;
use Text::Trim;
use feature "say";
use Getopt::Long;
use Utils::GeneralUse;
use List::MoreUtils qw(uniq);

# Check that all input files have same number of lines
sub check_equal_lines
{
    my $in_files_str = shift();
    my @in_files = split( ",", $in_files_str );
    my @lines = ();

    for my $f( @in_files )
    {
        my $output = `wc -l $f`;
        my @items = split( " ", $output );
        push( @lines, $items[0] );
    }

    my @uniq_lines = uniq( @lines );
    if( scalar( @uniq_lines ) == 1 )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

sub join_files
{
    my $in_files_str = shift();
    my $out_file = shift();
	my $params = shift();
	my $print_progress = fetch_param( $params, "print_progress", 0 );
    my $separator = fetch_param( $params, "separator", "," );
	print_subr_begin() if $print_progress;

    my $same_lines = check_equal_lines( $in_files_str );
    if( $same_lines==0 )
    {
        die "JoinFiles: Line counts in joining files are not equal";
    }

    my @in_files = split( ",", $in_files_str );
    my @fins = ();
    for my $f( @in_files )
    {
        my $fin = safe_open( "<:encoding( UTF-8 )", $f );
        push( @fins, $fin );
    }
    my $fout = safe_open( ">:encoding( UTF-8 )", $out_file );

    my $notdone = 1;
	my $linect=0;
    while( $notdone )
    {
		$linect = print_loop_progress( $linect, 10000 ) if $print_progress;

        my @lines = ();
        for my $fin( @fins )
        {
            if( !eof( $fin ) )
            {
                my $line = <$fin>;
                chomp( $line );
                if( $line ne "" ) # IGNORES EMPTY LINES
                {
                    push( @lines, $line );
                }
            }
            else
            {
                $notdone = 0;
            }
        }

        if( $notdone )
        {
            my $joined_line = join( $separator, @lines );
            print $fout "$joined_line\n";
        }
    }

    for my $fin ( @fins )
    {
        close( $fin );
    }
    close( $fout );

	print_subr_end() if $print_progress;
}

unless( caller )
{
    my $in_files_str = shift();
    my $out_file = shift();
	my %params;
	GetOptions( \%params, 'separator:s','print_progress:i' );
	@ARGV = ();
	join_files( $in_files_str, $out_file, \%params );
}