# usage: cat [cleaned file name] | perl words_2_spmat.pl [splitter] [words file name] [0/1, 0=>training data, 1=>test data]  > [sparse mat file name]
#        [splitter] is usually " ", but can be anything

use strict;
use warnings;

use File::Temp qw(tempfile);

my $splitter = $ARGV[0];
my $words_file = $ARGV[1];
my $train_or_test = $ARGV[2];
@ARGV = ();

my %words = ();
my $linect = 0;
my ($ftmp,$tmpname) = tempfile();
while(<STDIN>)
{
	print $ftmp "$_";

	chomp($_);
	$words{$_}++ for(split($splitter,$_));	 # assumes that words are separated by single space
	$linect++;
	#print "$linect\n" if($linect%1000==0);
}
close($ftmp);

my @sortwords;
if( $train_or_test==0 ) #training
{
	my $fwords; open($fwords, ">", $words_file);
	@sortwords = sort {$a cmp $b} keys(%words);
	print $fwords "$_\n" for(@sortwords);
	close($fwords);
}
elsif( $train_or_test==1 ) #test
{
	my $fwords; open($fwords, "<", $words_file);
	while( <$fwords> )
	{
		chomp( $_ );
		push( @sortwords, $_ );
	}
	close($fwords);
}

{
	%words = ();
	my $ctr = 0; 
	$words{$_} = $ctr++ for( @sortwords );
}

print "$linect ".scalar(keys(%words))."\n";

$linect = 0;
open($ftmp, "<", $tmpname);
while(<$ftmp>)
{
	chomp($_);
	my %wcounts = ();
	$wcounts{$_}++ for(split($splitter,$_));
	print join(" ", map {"$words{$_}:$wcounts{$_}"} (sort {$a cmp $b} ( grep { exists( $words{$_} ) } keys(%wcounts) ) )  )."\n";

	$linect++; 
	#print "$linect\n" if($linect%1000==0);
}
close($ftmp);

