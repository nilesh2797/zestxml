use strict;
use warnings;

sub get_dims( $ ) {
	my $fil = shift;
	#print "$fil\n";
	
	my $ninst=0;
	my $nft=0;
	my $nlbl=0;
	my $fin; open( $fin, "<", $fil );

	while( <$fin> )
	{
		$ninst++;

		my @items = split( " ", $_, 2 );

		if( $_ !~ m/^ .*$/ )
		{
			my @lbls = split( ",", $items[0] );
			for my $lbl( @lbls )
			{
				$nlbl = $lbl if( $lbl > $nlbl );
			}
		}

		my @fts = split( " ", $items[1] );
		for my $ft( @fts )
		{
			my @pair = split( ":", $ft, 2 );
			$nft = $pair[0] if( $pair[0] > $nft );
		}
	}

	close( $fin );

	return ($ninst,$nft,$nlbl);
}

sub write_fil( $$$ )
{
	my $in_file = shift;
	my $fft = shift;
	my $flbl = shift;

	my $fin; open( $fin, "<", $in_file );

	while( <$fin> )
	{
		my @items = split( " ", $_, 2 );

		if( $_ !~ m/^ .*$/ )
		{
			my @lbls = split( ",", $items[0] );
			@lbls = sort {$a <=> $b} @lbls;
			@lbls = map {my $l = $_-1; "$l:1"} @lbls;
			print $flbl join( " ", @lbls )."\n";		
		}
		else
		{
			print $flbl "\n";
		}

		my @fts = split( " ", $items[1] );
		@fts = map {my @pair = split( ":", $_ ); $pair[0]--; \@pair } @fts;
		#@fts = map {my @pair = split( ":", $_ ); $pair[0]--; "$pair[0]:$pair[1]" } @fts;
		@fts = sort {@$a[0] <=> @$b[0]} @fts;
		@fts = map {my @p = @$_; "$p[0]:$p[1]"} @fts;
		print $fft join( " ", @fts )."\n";
	}

	close( $fin );
}

my $in_trn = $ARGV[0];
my $in_tst = $ARGV[1];

my ($num_trn,$trn_num_ft,$trn_num_lbl) = get_dims( $in_trn );
my ($num_tst,$tst_num_ft,$tst_num_lbl) = get_dims( $in_tst );

print "$num_trn $trn_num_ft $trn_num_lbl\n";
print "$num_tst $tst_num_ft $tst_num_lbl\n";

my $num_ft =  $trn_num_ft>$tst_num_ft ? $trn_num_ft : $tst_num_ft;
my $num_lbl = $trn_num_lbl>$tst_num_lbl ? $trn_num_lbl : $tst_num_lbl;

print "$num_ft $num_lbl\n";

my $fft; open( $fft, ">", "X_Xf.txt" );
my $flbl; open( $flbl, ">", "X_Y.txt" );

my $num_inst = $num_trn + $num_tst;

print $fft "$num_inst $num_ft\n";
print $flbl "$num_inst $num_lbl\n";

write_fil( $in_trn, $fft, $flbl );
write_fil( $in_tst, $fft, $flbl );

close( $fft );
close( $flbl );

my $fsplit; open( $fsplit, ">", "split.0.txt" );
for my $i( 1..$num_trn )
{
	print $fsplit "0\n";
}
for my $i( 1..$num_tst )
{
	print $fsplit "1\n";
}
close( $fsplit );
