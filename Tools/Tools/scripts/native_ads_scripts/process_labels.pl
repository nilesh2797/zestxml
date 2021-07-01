use strict;
use warnings;

my $X_file = $ARGV[0];
my $Y_file = $ARGV[1];
my $in_file = $ARGV[2];
my $cl_file = $ARGV[3];
my $im_file = $ARGV[4];
my $ctr_file = $ARGV[5];

my $fin;
my $fout;
my $c;

my %Y = ();
$c = 0;
open( $fin, "<", $Y_file );
while( <$fin> )
{
	chomp( $_ );
	$Y{ $_ } = $c;
	$c++;
}
close( $fin );

my %labels = ();

open( $fin, "<", $in_file );
while( <$fin> )
{
	chomp($_);
	my @items = split( "\t", $_ );
	my $query = $items[0];
	my $uid = $items[1];
	my $cl = $items[2];
	my $im = $items[3];
	my $ctr;

	if( $im<10 )
	{
		$ctr = 0.01;
	}
	else
	{
		$ctr = $cl/$im;
		$ctr = sprintf( "%.4f", $ctr );
	}
	my $qid = $Y{ $query };

	if( not(exists($labels{$uid})) )
	{
		$labels{$uid} = ();
	}

	push( @{$labels{$uid}}, "$query:$cl:$im:$ctr" );
}
close( $fin );

my $fcl; open( $fcl, ">", $cl_file );
my $fim; open( $fim, ">", $im_file );
my $fctr; open( $fctr, ">", $ctr_file );

$c = 0;
open( $fin, "<", $X_file );
while( <$fin> )
{
	$c++;

	chomp($_);
	if( exists($labels{$_}) )
	{
		my @vec = @{$labels{$_}};
		my %dict = ();
		for my $v( @vec )
		{
			my @items = split( ":", $v, 2 );
			$dict{ $Y{ $items[0] } } = $items[1];
		}

		my @cls = ();
		my @ims = ();
		my @ctrs = ();

		for my $qid( sort {$a <=> $b} keys( %dict ) )
		{
			my $val = $dict{ $qid };
			my @items = split( ":", $val );
			push( @cls, "$qid:$items[0]" );
			push( @ims, "$qid:$items[1]" );
			push( @ctrs, "$qid:$items[2]" );
		}

		print $fcl join( " ", @cls )."\n";
		print $fim join( " ", @ims )."\n";
		print $fctr join( " ", @ctrs )."\n";
	}
	else
	{
		print "$_\n";
	}
}
close( $fin );
close( $fcl );
close( $fim );
close( $fctr );