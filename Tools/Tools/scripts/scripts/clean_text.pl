use strict;
use warnings;
use Data::Dumper;

use Lingua::StopWords qw(getStopWords);
my $stopwords = getStopWords(qw(en));
my %stopword_hash = %$stopwords;

use Lingua::Stem::Snowball;
my $stemmer = Lingua::Stem::Snowball->new( lang => "en" );

#print Dumper(%stopword_hash)."\n";

while(<STDIN>)
{
	chomp($_);
	$_ = lc($_);  # convert all text to lower case
	$_ =~ s/[^\w]+/ /g;  # replace all contiguous non-alphanumeric substrings with a single space
	my @words = split(" ", $_);
	@words = grep {not exists $stopword_hash{$_}} @words; # remove stop words
	$stemmer->stem_in_place(\@words); # perform stemming

	$_ = join(" ", @words);
	print "$_\n";
}
