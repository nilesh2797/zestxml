#!/usr/bin/env perl
package Utils::TextProcessing::TokenizeText;
use Exporter 'import';
our @EXPORT = qw(tokenize_text_fhandles tokenize_text_files); 

use strict;
use warnings;
use Data::Dumper;
use Text::Trim;
use feature "say";
use Getopt::Long;
use Utils::GeneralUse;
use File::Temp qw( tempfile );
use List::Util qw(min max);
use List::MoreUtils qw(uniq);

sub tokenize_text_fhandles
{
	my $fin = shift();
	my $fout = shift();
	my $params = shift();

    my $max_ngram_len = fetch_param( $params, "max_ngram_len", 2 );
    my $max_explore_ngram_len = fetch_param( $params, "max_explore_ngram_len", $max_ngram_len ); # since there is also an option to include mandatory tokens (must_retain_token_file), this allows wider exploration of ngrams
    my $min_token_freq = fetch_param( $params, "min_token_freq", 2 );
    my $max_token_freq = fetch_param( $params, "max_token_freq", 200 );
    my $retain_full_text_token = fetch_param( $params, "retain_full_text_token", 0 );
    my $must_retain_token_file = fetch_param( $params, "must_retain_token_file", "" );
   	my $print_progress = fetch_param( $params, "print_progress", 0 );


    print( STDERR "max_ngram_len: $max_ngram_len \n");
    print( STDERR "max_explore_ngram_len: $max_explore_ngram_len \n");
    print( STDERR "min_token_freq: $min_token_freq \n");
    print( STDERR "max_token_freq: $max_token_freq \n");
    print( STDERR "retain_full_text_token: $retain_full_text_token \n");
    print( STDERR "must_retain_token_file: $must_retain_token_file \n");

	print_subr_begin() if $print_progress;

    my %must_retain_tokens = ();
    if( $must_retain_token_file ne "" )
    {
        my $fm; open( $fm, "<", $must_retain_token_file );
        while( <$fm> )
        {
            chomp($_);
            $must_retain_tokens{ $_ } = 1;
        }
        close( $fm );
    }

    my %token_freqs = ();
    my ($fprefilter, $prefilter_file) = tempfile();
	my $linect=0;
    my $fulltokens = 0;
	while( <$fin> )
	{
		$linect = print_loop_progress( $linect, 10000 ) if $print_progress;
        my @tokens = ();
        chomp( $_ );
        my @items = split( " ", $_ );
        my $len = scalar( @items );

        for my $i( 0..(min($max_explore_ngram_len,$len)-1) )
        {
            for my $j( 0..($len-$i-1) )
            {
                my $k = $j+$i;
                my $substring = join( "_", @items[$j .. $k] );
                push( @tokens, $substring );
            }
        }

        if( $retain_full_text_token )
        {
            my $full_label_token = "FULL_".join( "_", @items )."_$fulltokens";
            $fulltokens++;
            push( @tokens, $full_label_token );
        }

        $token_freqs{ $_ }++ for( uniq( @tokens ) );
        say $fprefilter join( " ", @tokens );
    }
    close( $fprefilter );

    print( STDERR "fulltokens: \n$fulltokens\n" );

    $fprefilter = safe_open( "<", $prefilter_file );
    $linect=0;
    while( <$fprefilter> )
    {
        $linect = print_loop_progress( $linect, 10000 ) if $print_progress;
        chomp( $_ );
        my @tokens = split( " ", $_ );
        @tokens = grep { $_ =~ m/FULL_.*/ or exists( $must_retain_tokens{ $_ } ) or (scalar( split("_", $_ ) )<=$max_ngram_len and $token_freqs{$_}>=$min_token_freq and $token_freqs{$_}<=$max_token_freq) } @tokens;
        say $fout join( " ", @tokens );
    }
    close( $fprefilter );

	print_subr_end() if $print_progress;
}

sub tokenize_text_files
{
	my $in_file = shift();
	my $out_file = shift();
	my $params = shift();

	my $fin = safe_open( "<", $in_file );
	my $fout = safe_open( ">", $out_file );
	tokenize_text_fhandles( $fin, $fout, $params );
	close( $fin );
	close( $fout );
}

unless( caller )
{
	my %params;
	GetOptions( \%params, 'max_ngram_len:i','max_explore_ngram_len:i','min_token_freq:i','max_token_freq:i','retain_full_text_token:i','must_retain_token_file:s','print_progress:i' );
	tokenize_text_fhandles( *STDIN, *STDOUT, \%params );
}