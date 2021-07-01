use strict;
use warnings;

my $in_ftfile = $ARGV[0]; my $fin_ft; open( $fin_ft, "<", $in_ftfile ) or die $!;
my $in_lblfile = $ARGV[1]; my $fin_lbl; open( $fin_lbl, "<", $in_lblfile );
my $in_splitfile = $ARGV[2]; my $fin_split; open( $fin_split, "<", $in_splitfile );
my $out_trnfile = $ARGV[3]; my $fout_trn; open( $fout_trn, ">", $out_trnfile );
my $out_tstfile = $ARGV[4]; my $fout_tst; open( $fout_tst, ">", $out_tstfile );

$_ = <$fin_ft>;
$_ = <$fin_lbl>;

while( my $split_line = <$fin_split> )
{
	chomp( $split_line );
	
	my $ft_line = <$fin_ft>;
	chomp( $ft_line );
	my $lbl_line = <$fin_lbl>;
	chomp( $lbl_line );

	my @fts = split( " ", $ft_line );
	@fts = map {my @pair = split( ":", $_ ); $pair[0]++; \@pair } @fts;
	my @lbls = split( " ", $lbl_line );
	@lbls = map {my @pair = split( ":", $_ ); $pair[0]++; $pair[0] } @lbls;

	my $outline = join( ",", @lbls )." ".join( " ", map {"@$_[0]:@$_[1]"} @fts );

	if( $split_line == 0 )
	{
		print $fout_trn "$outline\n";
	}
	else
	{
		print $fout_tst "$outline\n";
	}
}

close( $fin_ft );
close( $fin_lbl );
close( $fin_split );
close( $fout_trn );
close( $fout_tst );

