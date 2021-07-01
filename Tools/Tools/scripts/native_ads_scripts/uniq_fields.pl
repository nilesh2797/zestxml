use strict;
use warnings;

my $field_num = $ARGV[0];
my $sep = $ARGV[1];
@ARGV = ();

my %fields = ();

while( <STDIN> )
{
	chomp($_);
	my @items = split( $sep, $_ );
	my $field = $items[$field_num-1];
	if( $field ne "" )
	{
		$fields{ $field }++;
	}
}

my @fieldarr = sort {$a cmp $b} keys( %fields );
for my $f( @fieldarr )
{
	print "$f\n";
}