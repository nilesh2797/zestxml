use strict;
use warnings;

my $infile = $ARGV[0];
my $fin; open( $fin, "<", $infile );

my $outfile = $ARGV[1];
my $fout; open( $fout, ">", $outfile );

my $count = 0;
my $sum = 0;

while( <$fin> )
{
	chomp( $_ );
	my @items = split( "\t", $_ );
	$sum += $items[1];
	$count++;

	if( $count % 100000 == 0 )
	{
		print $fout "$count\t$sum\n";
	}
}

close( $fin );
close( $fout );