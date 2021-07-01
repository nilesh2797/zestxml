#!/usr/bin/env perl
# usage from shell: cat [json data file] | ExtractJSONFields.pm [output folder] [comma separated list of fields to be extracted] --unaccent [0/1, 1=>convert all accented letters to unaccented ascii] --print_progress [0/1]

package Utils::TextProcessing::ExtractJSONFields;
use Exporter 'import';
our @EXPORT_OK = qw(extract_json_fields_fhandles extract_json_fields_files); 

use strict;
use warnings;
use File::Temp qw(tempfile);
use Getopt::Long;
use Utils::GeneralUse;
use feature "say";
use JSON::Parse qw( parse_json );
use Text::Unaccent;

sub extract_json_fields_fhandles
{
	my $fjson = shift();
	my $output_dir = shift();
	my $required_fields_str = shift();
	my $params = shift();
	my $ascii_output = fetch_param( $params, "ascii_output", 0 );
	my $print_progress = fetch_param( $params, "print_progress", 0 );
	print_subr_begin() if $print_progress;

    my @required_fields = split( ",", $required_fields_str );
    my %field_fhandles = ();
    for my $f( @required_fields )
    {
        my $fout = safe_open( ">:encoding(UTF-8)", "$output_dir/$f.txt" );
        $field_fhandles{ $f } = $fout;
    }

	my $linect = 0;
	while( <$fjson> )
	{
		$linect = print_loop_progress( $linect, 10000 ) if $print_progress;
        chomp( $_ );
        $_ =~ s/\\n/ /g; 
        my %p = %{ parse_json( $_ ) }; 

        for my $f( @required_fields )
        {
            if( exists( $p{ $f } ) )
            {
                my $s;
                if( ref( $p{ $f } ) eq "ARRAY" ) # Note that if arrays, they will be printed in the same line in COMMA-SEPARATED format. Assuming that within any value there are no commas
                {
                    $s = join( ",", @{ $p{ $f } } );
                }
                else
                {
                    $s = $p{ $f };
                }
                
                if( $ascii_output )
                {
                    $s = unac_string( 'UTF-8', $s );
                    $s =~ s/[^[:ascii:]]/ /g;
                }

                print { $field_fhandles{ $f } } "$s\n";
            }
            else
            {
                print { $field_fhandles{ $f } } "\n";
            }
        }
	}

    for my $f( @required_fields )
    {
        close( $field_fhandles{ $f } );
    }
	
	print_subr_end() if $print_progress;
}

sub extract_json_fields_files
{
	my $json_data_file = shift();
	my $output_dir = shift();
	my $required_fields = shift();
	my $params = shift();

	my $fjson = safe_open( "<:encoding(UTF-8)", $json_data_file );
	extract_json_fields_fhandles( $fjson, $output_dir, $required_fields, $params );
	close( $fjson );
}

unless( caller )
{
    my $output_dir = shift();
    my $required_fields = shift();
	my %params;
	GetOptions( \%params, 'ascii_output:i', 'print_progress:i' );
	@ARGV = ();
	extract_json_fields_fhandles( *STDIN, $output_dir, $required_fields, \%params );
}