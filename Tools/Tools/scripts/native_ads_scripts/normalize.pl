use strict;
use warnings;

use Lingua::StopWords qw(getStopWords);
my $stopwords = getStopWords(qw(en));
my %stopword_hash = %$stopwords;

use Lingua::Stem::Snowball;
my $stemmer = Lingua::Stem::Snowball->new( lang => "en" );

my $infile = $ARGV[0];
my $fin; open( $fin, "<", $infile );

my $outfile = $ARGV[1];
my $fout; open( $fout, ">", $outfile );

my %dict = ();

while( <$fin> )
{
	chomp( $_ );
	my @items = split( "\t", $_ );
	my $inquery = $items[0];
	my $outquery = normalize( $inquery );
	$dict{ $outquery } += $items[1];
}

print $fout "$_\t$dict{$_}\n" for( sort {$dict{$b} <=> $dict{$a}} keys( %dict ) );


close( $fin );
close( $fout );


sub normalize
{
	my $inquery = $_[0];
	chomp( $inquery );
	$inquery = lc( $inquery );  # convert all text to lower case
	$inquery =~ s/[^\w]+/ /g;  # replace all contiguous non-alphanumeric substrings with a single space
	my @words = split(" ", $inquery);
	@words = grep {not exists $stopword_hash{$_}} @words; # remove stop words
	$stemmer->stem_in_place(\@words); # perform stemming
	@words = sort {$a cmp $b} @words;
	my $outquery = join(" ", @words);
	return $outquery;
}
