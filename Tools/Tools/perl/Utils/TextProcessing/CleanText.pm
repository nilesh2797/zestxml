#!/usr/bin/env perl
package Utils::TextProcessing::CleanText;
use Exporter 'import';
our @EXPORT = qw(clean_text_fhandles clean_text_files); 

use strict;
use warnings;
use Data::Dumper;
use Text::Trim;
use feature "say";
use Lingua::StopWords qw(getStopWords);
my $default_stopwords_hash = getStopWords(qw(en));
use Lingua::Stem::Snowball;
my $default_stemmer = Lingua::Stem::Snowball->new( lang => "en" );
use Utils::TextProcessing::PluralToSingular qw/to_singular/;
use Getopt::Long;
use Utils::GeneralUse;

sub clean_text_fhandles
{
	my $fin = shift();
	my $fout = shift();
	my $params = shift();
	my $has_index = fetch_param( $params, "has_index", 0 );
	my $index_sep = fetch_param( $params, "index_sep", "\t" );
	my $do_lowercase = fetch_param( $params, "do_lowercase", 1 );
	my $remove_stopwords = fetch_param( $params, "remove_stopwords", 1 );
	my %stopwords_hash = %{fetch_param( $params, "stopwords_hash", $default_stopwords_hash )};
	my $do_stemming = fetch_param( $params, "do_stemming", 1 );
	my $stemmer = fetch_param( $params, "stemmer", $default_stemmer );
	my $punctuation_to_commas = fetch_param( $params, "punctuation_to_commas", 0 );
	my $do_singular = fetch_param( $params, "do_singular", 0 );
	my $print_progress = fetch_param( $params, "print_progress", 0 );
	print_subr_begin() if $print_progress;

	my $linect=0;
	while( <$fin> )
	{
		$linect = print_loop_progress( $linect, 10000 ) if $print_progress;
		chomp($_);
		
		my $index;
		my $value;
		if( $has_index )
		{
			my @items = split( $index_sep, $_, 2 );
			$index = $items[0];
			$value = $items[1];
		}
		else
		{
			$value = $_;
		}

		if( $do_lowercase )
		{
			$value = lc( $value );  # convert all text to lower case
		}

		if( $punctuation_to_commas )
		{
			$value =~ s/[.,;:'-[]()!?]/,/g; # replace all punctuations to commas and retain them for properly demarcating ngrams
			$value =~ s/^0-9a-zA-Z,]+/ /g;
		}
		else
		{
			$value =~ s/[^0-9a-zA-Z]+/ /g;  # replace all contiguous non-alphanumeric substrings with a single space
		}
		my @words = split(" ", $value);

		if( $remove_stopwords )
		{
			@words = grep {not exists $stopwords_hash{$_}} @words; # remove stop words
		}

		if( $do_singular )
		{
			@words = map { to_singular( $_ ) } @words;
		}

		if( $do_stemming )
		{
			$stemmer->stem_in_place(\@words); # perform stemming
		}

		$value = join(" ", @words);

		if( $has_index )
		{
			say $fout "$index$index_sep$value";
		}
		else
		{
			say $fout "$value";
		}
	}
	print_subr_end() if $print_progress;
}

sub clean_text_files
{
	my $in_file = shift();
	my $out_file = shift();
	my $params = shift();

	my $fin = safe_open( "<", $in_file );
	my $fout = safe_open( ">", $out_file );
	clean_text_fhandles( $fin, $fout, $params );
	close( $fin );
	close( $fout );
}

unless( caller )
{
	my %params;
	GetOptions( \%params, 'has_index:i','index_sep:s','do_lowercase:i','remove_stopwords:i','stopwords_file:s','do_stemming:i','stemmer_type:s','do_singular:i','punctuation_to_commas:i','print_progress:i' );
	if( exists( $params{ "stopwords_file" } ) )
	{
		my %stopwords_hash = ();
		my $fin = safe_open( "<", $params{ "stopwords_file" } );
		while( <$fin> )
		{
			chomp( $_ );
			$stopwords_hash{ $_ } = 1;
		}
		close( $fin );
		$params{ "stopwords_hash" } = \%stopwords_hash;
	}
	if( exists( $params{ "stemmer_type" } ) )
	{
		if( $params{ "stemmer_type" } eq "snowball" )
		{	
			$params{ "stemmer" } = Lingua::Stem::Snowball->new( lang => "en" );
		}
		else
		{
			die "ERROR: Stemmer of type". $params{ "stemmer_type" } ."is not supported by the script yet\n";
		}
	}
	@ARGV = ();
	clean_text_fhandles( *STDIN, *STDOUT, \%params );
}