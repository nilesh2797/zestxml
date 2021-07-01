#!/usr/bin/env perl
package Utils::TextProcessing::GenIRMatchTokenFeatures;
use Exporter 'import';
our @EXPORT = qw(gen_irmatch_token_features_fhandles gen_irmatch_token_features_files); 

use strict;
use warnings;
use Data::Dumper;
use Text::Trim;
use feature "say";
use Getopt::Long;
use Utils::GeneralUse;
use File::Temp qw( tempfile );
use List::Util qw(min max);
use File::Copy qw(copy);
use Switch;

sub gen_irmatch_token_features_fhandles
{
    my $f_trn_X_per_Y = shift();
    my $f_Y_per_T = shift();
    my $f_trn_X_per_T = shift();
    my $f_T_precision = shift();
    my $f_proc_Y = shift();
	my $f_Y_T = shift();
	my $f_T = shift();
    my $f_Tf = shift();
    my $f_Y_T_out = shift();
    my $f_TfParam = shift();
	my $params = shift();
    my $max_depth = 5;

   	my $print_progress = fetch_param( $params, "print_progress", 1 );

	print_subr_begin() if $print_progress;

    ### Read and store TfParams
    my @TfParam = ();
    while( <$f_TfParam> )
    {
        chomp( $_ );
        my @items = split( "\t", $_ );
        #print $items[0]."\n";
        #print $items[1]."\n";

        my @vec = ();
        if( scalar(@items)==1 )
        {
            push( @vec, $items[0] );
            push( @vec, "NA" );
        }
        elsif( scalar(@items)==2 )
        {
            push( @vec, $items[0] );
            push( @vec, $items[1] );
        }
        else
        {
            die "Invalid line length in TfParam file";
        }
        push( @TfParam, \@vec );
    }

    ### Read and store tokens in %T
    my %T = ();
    my $count = 0;
    while( <$f_T> )
    {
        chomp( $_ );
        $T{ $count } = $_;
        $count++;
    }

    ### Read and store label frequencies w.r.t points (i.e. how many training points per label)
    my %trn_X_per_Y = ();
    $count = 0;
    while( <$f_trn_X_per_Y> )
    {
        chomp($_);
        $trn_X_per_Y{ $count } = $_;
        $count++;
    }

    ### Read and store token frequencies w.r.t points (i.e. how many training points per token) and labels (i.e. how many labels per token)
    my %trn_X_per_T = ();
    $count = 0;
    while( <$f_trn_X_per_T> )
    {
        chomp($_);
        $trn_X_per_T{ $T{$count} } = $_;
        $count++;
    }

    ### Read and store token precisions
    my %T_precision = ();
    $count = 0;
    while( <$f_T_precision> )
    {
        chomp($_);
        $T_precision{ $count } = $_;
        $count++;
    }

    my %Y_per_T = ();
    $count = 0;
    while( <$f_Y_per_T> )
    {
        chomp($_);
        $Y_per_T{ $T{$count} } = $_;
        $count++;
    }

    ### Create token features
    $_ = <$f_Y_T>;
    chomp( $_ );
    my ($num_Y,$num_T) = split( " ", $_ );
    print $f_Y_T_out "$num_Y $num_T\n";

    my %Tf = ();
    my $Tf_count = 0;

	my $linect=0;
    while( <$f_Y_T> )
    {
        $linect = print_loop_progress( $linect, 100 ) if $print_progress;

        my $label = <$f_proc_Y>;
        chomp( $label );
        my $label_freq = $trn_X_per_Y{ $linect-1 };

        my @tokens = ();
        chomp( $_ );
        my @token_indices = map {my @pair = split( ":", $_ ); $pair[0]} split( " ", $_ );
        for my $item ( @token_indices )
        {
            push( @tokens, $T{$item} );
        }
        #print join( " ", @tokens )."\n";

        my @puretokens = ();
        for my $token( @tokens )
        {
            my ($header,$puretoken) = split( "_", $token, 2 );
            push( @puretokens, $puretoken );
        }
        #print join( " ", @puretokens )."\n";

        my %token_features = ();
        for my $token( @tokens )
        {
            $token_features{ $token } = "";
        }

        for my $ft( @TfParam )
        {
            my @vec = @$ft;
            my $ftname = $vec[0];
            my @ftbuckets = split( ",", $vec[1] );

            $token_features{ $ftname } = ();

            switch( $ftname )
            {
                case "exact"
                {
                    for my $token( @tokens )
                    {
                        if( $token =~ m/FULL_.*/ )
                        {
                            $token_features{ $token } = "$token_features{ $token } 1";
                        }
                        else
                        {
                            $token_features{ $token } = "$token_features{ $token } 0";
                        }
                    }
                }
                case "prefix"
                {
                    my $count = 0;
                    for my $token( @tokens )
                    {
                        my $puretoken = $puretokens[$count++];
                        if( $label =~ m/^$puretoken.+$/ )
                        {
                            $token_features{ $token } = "$token_features{ $token } 1";
                        }
                        else
                        {
                            $token_features{ $token } = "$token_features{ $token } 0";
                        }
                    }
                }
                case "suffix"
                {
                    my $count = 0;
                    for my $token( @tokens )
                    {
                        my $puretoken = $puretokens[$count++];
                        if( $label =~ m/^.+$puretoken$/ )
                        {
                            $token_features{ $token } = "$token_features{ $token } 1";
                        }
                        else
                        {
                            $token_features{ $token } = "$token_features{ $token } 0";
                        }
                    }
                }
                case "infix"
                {
                    my $count = 0;
                    for my $token( @tokens )
                    {
                        my $puretoken = $puretokens[$count++];
                        if( $label =~ m/^.+$puretoken.+$/ )
                        {
                            $token_features{ $token } = "$token_features{ $token } 1";
                        }
                        else
                        {
                            $token_features{ $token } = "$token_features{ $token } 0";
                        }
                    }
                }
                case "tokenprecision"
                {
                    my $tc = 0;
                    for my $token( @tokens )
                    {
                        my $val = $T_precision{ $token_indices[$tc] };
                        $tc++;
                        my $c = 0;
                        for my $bucket( @ftbuckets )
                        {
                            if( $val <= $bucket )
                            {
                                $token_features{ $token} = "$token_features{ $token } $c";
                                last;
                            }
                            else
                            {
                                $c++;
                            }
                        }
                        if( $c==scalar(@ftbuckets) )
                        {
                            $token_features{ $token} = "$token_features{ $token } $c";
                        }
                    }
                }
                case "pointsperlabel"
                {
                    for my $token( @tokens )
                    {
                        my $val = $label_freq;
                        my $c = 0;
                        for my $bucket( @ftbuckets )
                        {
                            if( $val <= $bucket )
                            {
                                $token_features{ $token} = "$token_features{ $token } $c";
                                last;
                            }
                            else
                            {
                                $c++;
                            }
                        }
                        if( $c==scalar(@ftbuckets) )
                        {
                            $token_features{ $token} = "$token_features{ $token } $c";
                        }
                    }
                }
                case "labelspertoken"
                {
                    for my $token( @tokens )
                    {
                        my $val = $Y_per_T{ $token };
                        my $c = 0;
                        for my $bucket( @ftbuckets )
                        {
                            if( $val <= $bucket )
                            {
                                $token_features{ $token} = "$token_features{ $token } $c";
                                last;
                            }
                            else
                            {
                                $c++;
                            }
                        }
                        if( $c==scalar(@ftbuckets) )
                        {
                            $token_features{ $token} = "$token_features{ $token } $c";
                        }
                    }
                }
                case "pointspertoken"
                {
                    for my $token( @tokens )
                    {
                        my $val = $trn_X_per_T{ $token };
                        my $c = 0;
                        for my $bucket( @ftbuckets )
                        {
                            if( $val <= $bucket )
                            {
                                $token_features{ $token} = "$token_features{ $token } $c";
                                last;
                            }
                            else
                            {
                                $c++;
                            }
                        }
                        if( $c==scalar(@ftbuckets) )
                        {
                            $token_features{ $token} = "$token_features{ $token } $c";
                        }
                    }
                }
                case "ancestorcount"
                {
                    #print join( " ", @puretokens )."\n";

                    my $num_token = scalar( @tokens );
                    my %depths = ();
                    for my $ptoken( @tokens )
                    {
                        $depths{ $ptoken } = 1;
                    }
                    my %descendants = ();
                    for my $token( @tokens )
                    {
                        @{$descendants{ $token }} = ();
                    }
                    for my $i( 0..($num_token-1) )
                    {
                        for my $j( ($i+1)..($num_token-1) )
                        {
                            my $ptokeni = $puretokens[ $i ];
                            my $ptokenj = $puretokens[ $j ];
                            my $tokeni = $tokens[ $i ];
                            my $tokenj = $tokens[ $j ];

                            if( $ptokeni ne $ptokenj )
                            {
                                if( index($ptokeni, $ptokenj) != -1 )
                                {
                                    #print "$i $tokeni $j $tokenj\n";
                                    push( @{$descendants{ $tokeni }}, $tokenj );
                                }
                                elsif( index($ptokenj, $ptokeni) != -1 )
                                {
                                    #print "$j $tokenj $i $tokeni\n";
                                    push( @{$descendants{ $tokenj }}, $tokeni );
                                }
                            }
                        }
                    }
                    for my $ptoken( sort { scalar(@{$descendants{$b}}) <=> scalar(@{$descendants{$a}}) } @tokens )
                    {
                        #print "$ptoken\n";
                        for my $ptokend( @{$descendants{ $ptoken }} )
                        {
                            #print "\t$ptokend\n";
                            $depths{ $ptokend } = $depths{ $ptoken } + 1;
                        }
                    }
                    for my $token( @tokens )
                    {
                        $token_features{$token} = "$token_features{ $token } $depths{$token}";
                    }
                }
                case "coverage"
                {
                    my $count = 0;
                    for my $token( @tokens )
                    {
                        my $puretoken = $puretokens[$count++];
                        my $num = scalar( split( "_", $puretoken ) );
                        my $den = scalar( split( "_", $label ) );
                        $den=1 if($den==0);
                        my $val =  $num/$den ;
                        #print "$label\t$puretoken\tcoverage: $val\n";
                        my $c = 0;
                        for my $bucket( @ftbuckets )
                        {
                            if( $val <= $bucket )
                            {
                                $token_features{ $token} = "$token_features{ $token } $c";
                                last;
                            }
                            else
                            {
                                $c++;
                            }
                        }
                        if( $c==scalar(@ftbuckets) )
                        {
                            $token_features{ $token} = "$token_features{ $token } $c";
                        }
                    }
                }
                case "wordlength"
                {
                    my $count = 0;
                    for my $token( @tokens )
                    {
                        my $puretoken = $puretokens[$count++];
                        my $val =  scalar( split("_", $puretoken ) ) ;
                        my $c = 0;
                        for my $bucket( @ftbuckets )
                        {
                            if( $val <= $bucket )
                            {
                                $token_features{ $token} = "$token_features{ $token } $c";
                                last;
                            }
                            else
                            {
                                $c++;
                            }
                        }
                        if( $c==scalar(@ftbuckets) )
                        {
                            $token_features{ $token} = "$token_features{ $token } $c";
                        }
                    }
                }
                case "charlength"
                {
                    my $count = 0;
                    for my $token( @tokens )
                    {
                        my $puretoken = $puretokens[$count++];
                        $puretoken =~ s/_//g;
                        my $val =  length( $puretoken );
                        #print "$puretoken\t$val\n";
                        my $c = 0;
                        for my $bucket( @ftbuckets )
                        {
                            if( $val <= $bucket )
                            {
                                $token_features{ $token} = "$token_features{ $token } $c";
                                last;
                            }
                            else
                            {
                                $c++;
                            }
                        }
                        if( $c==scalar(@ftbuckets) )
                        {
                            $token_features{ $token} = "$token_features{ $token } $c";
                        }
                    }
                }
                else
                {
                    print "Unknown token feature type $ftname\n";
                }
            }
        }

        my @Y_T_out_vec = ();
        my $count = 0;
        for my $token( @tokens )
        {
            my $token_index = $token_indices[ $count++ ];
            #print "$label\t\t$token\t\t".join( ",",  $token_features{$token} )."\n";
            my $token_feature_str = $token_features{$token};
            if( !exists( $Tf{ $token_feature_str } ) )
            {
                $Tf{ $token_feature_str } = $Tf_count;
                $Tf_count++;
            }
            push( @Y_T_out_vec, "$token_index:$Tf{ $token_feature_str }" );
        }
        print $f_Y_T_out join( " ", @Y_T_out_vec )."\n";
    }
	print_subr_end() if $print_progress;

    my $total_feats = 0;
    for my $ft( @TfParam )
    {
        my @vec = @$ft;
        my $ftname = $vec[0];
        my @ftbuckets = split( ",", $vec[1] );
        if( $ftbuckets[0] eq "NA" )
        {
           $total_feats++;
        }
        else
        {
            $total_feats += (scalar( @ftbuckets )+1);
        }
    }

    print $f_Tf scalar(keys(%Tf))." ".$total_feats."\n";

    for my $s( sort { $Tf{$a} <=> $Tf{$b} } keys(%Tf) )
    {
        trim( $s );
        my @infeats = split( " ", $s );
        my @outfeats = ();
        my $count = 0;
        my $c = 0;
        for my $ft( @TfParam )
        {
            my @vec = @$ft;
            my $ftname = $vec[0];
            my @ftbuckets = split( ",", $vec[1] );
            if( $ftbuckets[0] eq "NA" )
            {
                if( $infeats[$c]==1 )
                {
                    push( @outfeats, "$count:1" );
                }
                $count++;
            }
            else
            {
                push( @outfeats, ($count+$infeats[$c]).":1" );
                $count += (scalar( @ftbuckets )+1);
            }
            $c++;
        }
        #print "$s\n";
        print $f_Tf join( " ", @outfeats)."\n";
    }
    print scalar( keys( %Tf ) )."\n";
}

sub gen_irmatch_token_features_files
{
    my $trn_X_per_Y_file = shift();
    my $Y_per_T_file = shift();
    my $trn_X_per_T_file = shift();
    my $T_precision_file = shift();
    my $proc_Y_file = shift();
	my $Y_T_file = shift();
	my $T_file = shift();
    my $Tf_file = shift();
    my $Y_T_out_file = shift();
    my $TfParam_file = shift();
	my $params = shift();

    my $f_trn_X_per_Y = safe_open( "<", $trn_X_per_Y_file );
    my $f_Y_per_T = safe_open( "<", $Y_per_T_file );
    my $f_trn_X_per_T = safe_open( "<", $trn_X_per_T_file );   
    my $f_T_precision = safe_open( "<", $T_precision_file ); 
    my $f_proc_Y = safe_open( "<", $proc_Y_file );
	my $f_Y_T = safe_open( "<", $Y_T_file );
	my $f_T = safe_open( "<", $T_file );
    my $f_Tf = safe_open( ">", $Tf_file );
    my $f_Y_T_out = safe_open( ">", $Y_T_out_file );
    my $f_TfParam = safe_open( "<", $TfParam_file );

	gen_irmatch_token_features_fhandles( $f_trn_X_per_Y, $f_Y_per_T, $f_trn_X_per_T, $f_T_precision, $f_proc_Y, $f_Y_T, $f_T, $f_Tf, $f_Y_T_out, $f_TfParam, $params );

    close( $f_trn_X_per_Y );
    close( $f_Y_per_T );
    close( $f_trn_X_per_T );
    close( $f_T_precision );
    close( $f_proc_Y );
	close( $f_Y_T );
	close( $f_T );
    close( $f_Tf );
    close( $f_Y_T_out );
    close( $f_TfParam );
}

unless( caller )
{
	my %params;
	GetOptions( \%params, 'print_progress:i' );
    my $trn_X_per_Y_file = $ARGV[0];
    my $Y_per_T_file = $ARGV[1];
    my $trn_X_per_T_file = $ARGV[2];
    my $T_precision_file = $ARGV[3];
    my $proc_Y_file = $ARGV[4];
    my $Y_T_file = $ARGV[5];
    my $T_file = $ARGV[6];
    my $Tf_file = $ARGV[7];
    my $TfParam_file = $ARGV[8];
    my ($f,$Y_T_out_file) = tempfile();
    print "$Y_T_out_file\n";
	gen_irmatch_token_features_files( $trn_X_per_Y_file, $Y_per_T_file, $trn_X_per_T_file, $T_precision_file, $proc_Y_file, $Y_T_file, $T_file, $Tf_file, $Y_T_out_file, $TfParam_file, \%params );
    copy( $Y_T_out_file, $Y_T_file );
}