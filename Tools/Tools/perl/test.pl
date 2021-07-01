use Utils::TextProcessing::TextSQL qw(inner_join_on_first_fields aggregate_on_first_field);

my $fin;
open( $fin, "<", "test.txt" );
aggregate_on_first_field( $fin, \*STDOUT, "\t", "," );
close( $fin );

my $f1;
open( $f1, "<", "test1.txt" );
my $f2;
open( $f2, "<", "test2.txt" );
inner_join_on_first_fields( $f1, $f2, \*STDOUT, "\t" );
close( $f1 );
close( $f2 );