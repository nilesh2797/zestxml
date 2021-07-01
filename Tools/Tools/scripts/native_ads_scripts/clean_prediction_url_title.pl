use strict;
use warnings;

my $raw_file = $ARGV[0];
my $X_file = $ARGV[1];
my $tokens_file = $ARGV[2];

my $fX;
open( $fX, ">", $X_file );
my $ftokens;
open( $ftokens, ">", $tokens_file );

my $url;
my $title;

my $fin;
open( $fin, "<", $raw_file );
my $ctr = 0;
while( <$fin> )
{
	chomp($_);
	if( $_ =~ m/^URL:\s*(.*)$/ )
	{
		$url = $1;
	}
	if( $_ =~ m/^Title:\s*(.*)$/ )
	{
		$title = $1;
		print $fX "$url\t$title\n";
		my $text = "$url $title";
		print $ftokens "$text\n";
	}
	
}
close( $fin );

close( $fX );
close( $ftokens );
