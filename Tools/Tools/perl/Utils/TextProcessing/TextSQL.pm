#!/usr/bin/perl
package Utils::TextProcessing::TextSQL;

use strict;
use warnings;
use Text::Trim;
use feature "say";
use Exporter 'import';
our @EXPORT_OK = qw(inner_join_on_first_fields aggregate_on_first_field); 

# Reads next line of file with format <id><sep><val> and returns an array of (id,val). If id and val fields are not available, returns empty fields
sub read_index_value_line
{
  my $fin = shift();
  my $sep = shift();
  my $line = <$fin>;
	$line or return;
	trim($line);
	my @items = split( $sep, $line, 2 );
  my $len_items = scalar( @items );

  if( $len_items ==2 )
  {
    return @items;
  }
  elsif( $len_items<2 )
  {
    say STDERR "Line contains < 2 fields, returning (\"\",\"\")";
    return ("","");
  }
}

# Reads a file with each line of format <id><idvalsep><val> where <id> in successive lines are non-decreasing, aggregates all <val> corresponding to the same <id> and outputs one line for each <id> in the format <id><idvalsep><val1><valvalsep><val2>...<valn>
# E.g. "0\ta\n0\tb\n1\tc" => "0\ta,b\n1\tc"
sub aggregate_on_first_field
{
  my $fin = shift();
  my $fout = shift();
  my $idvalsep = shift();
  my $valvalsep = shift();
  my $curr = ""; 
  my $str = "";

  while( <$fin> )
  {
    trim($_);
    my @fields = split( $idvalsep, $_, 2 );
  
    if( $curr eq "" )
    {
      $curr = $fields[0];
      $str = $fields[1];
    }
    elsif( $curr eq $fields[0] )
    {
      $str .= "$valvalsep$fields[1]";
    }
    else
    {
      say $fout "$curr$idvalsep$str";
      $curr = $fields[0];
      $str = $fields[1];
    }
  }
}

# Reads 2 files with each line of format <id><sep><val> where <id> in successive lines are non-decreasing, performs inner join on <id> in the 2 files, and outputs <id><sep><val1><sep><val2> line
sub inner_join_on_first_fields
{
  my $f1 = shift();
  my $f2 = shift();
  my $fout = shift();
  my $sep = shift();

  my $first = "-1";
  my $second = "-2";
  my $first_text = "";
  my $second_text = "";
  my @items;

  while(1)
  {
    if($first < $second)
    {
      (@items = read_index_value_line( $f1, $sep )) or last;
      ($first,$first_text) = @items;
    }
    elsif($first > $second)
    {
      (@items = read_index_value_line( $f2, $sep )) or last;
      ($second,$second_text) = @items;
    }
    else
    {
      say $fout "$first$sep$first_text$sep$second_text";
      (@items = read_index_value_line( $f1, $sep )) or last;
      ($first,$first_text) = @items;
      (@items = read_index_value_line( $f2, $sep )) or last;
      ($second,$second_text) = @items;
    }
  }
}

1;