use strict;
use warnings;

my $infile1 = $ARGV[0];
my $infile2 = $ARGV[1];

my %dict = ();

my $fin;
open( $fin, "<", $infile1 );
while( <$fin> )
{
	chomp($_);
	$dict{ $_ } = 1;
}
close( $fin );

open( $fin, "<", $infile2 );
while( <$fin> )
{
	chomp( $_ );
	if( exists( $dict{$_} ) )
	{
		print "$_\n";
	}
}
close( $fin );