use strict;
use warnings;

my $X_file = $ARGV[0];
my $in_feat_file = $ARGV[1];
my $out_feat_file = $ARGV[2];

my %dict = ();
my $fin;
open( $fin, "<", $in_feat_file );
while( <$fin> )
{
	chomp( $_ );
	my @items = split( "\t", $_ );
	$dict{ $items[0] } = $items[1];
}
close( $fin );

open( $fin, "<", $X_file );
my $fout;
open( $fout, ">", $out_feat_file );
while( <$fin> )
{
	chomp( $_ );
	print $fout "$dict{ $_ }\n";
}
close( $fin );
close( $fout );