use strict;
use warnings;

my %dict = ();
my $totrevenue = 0;

my $infile = $ARGV[0];
my $fin; 
open( $fin, "<", $infile );

while( <$fin> )
{
	chomp( $_ );
	my @items = split( "\t", $_ );
	$dict{ $items[0] } = $items[1];
	$totrevenue += $items[1];
}

close( $fin );

$infile = $ARGV[1];
open( $fin, "<", $infile );

my $revenue = 0;
my $found = 0;
my $absent = 0;

while( <$fin> )
{
	chomp( $_ );
	my @items = split( "\t", $_ );
	if( exists( $dict{$items[0]} ) )
	{
		print "$items[0]\t$dict{$items[0]}\n";
		$revenue += $dict{$items[0]};
		$found++;
	}
	else
	{
		print "$items[0]\t-1\n";
		$absent++;
	}
}

close( $fin );

print "totrevenue: $totrevenue\n";
print "revenue: $revenue\n";
print "found: $found\n";
print "absent: $absent\n";