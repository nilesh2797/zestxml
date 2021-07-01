package Utils::TextProcessing::ProcessWikiDump;

use strict;
use warnings;
use Text::Trim;
use feature "say";
use Utils::TextProcessing::TextSQL;

sub process_category_links_file
{
	my $fin_name = shift();
	my $fin; open( $fin, "<", $fin_name );
	my $fout_page_name = shift();
	my $fout_page; open( $fout_page, ">", $fout_page_name );
	my $fout_cat_name = shift();
	my $fout_cat; open( $fout_cat, ">", $fout_cat_name );

	while( <$fin> )
	{
		chomp( $_ );
		while( $_ =~ m/\(([0-9]*),'([^']*)',(?:'[^']*',){4}'(page|subcat)'\)/g )
		{
			my $str = $2;
			my $one = $1;
			my $three = $3;
			
			$str =~ s/@@/\\'/g;

			if( $three eq "page" )
			{
				say $fout_page "$one\t'$str'";
			}
			else
			{
				say $fout_cat "$one\t'$str'";
			}
		}
	}

	close( $fin );
	close( $fout_page );
	close( $fout_cat );
}

sub process_page_titles_file_line
{
	chomp( $_ );
	while( $_ =~ m/\(([0-9]*),([0-9]*),'([^']*)','([^']*)',([0-9]*),([0-9]*),[^()]*\)/g )
	{
		if(($2 eq "0" or $2 eq "14") and $5 eq "0")
		{
			say "$1\t$3";
		}
	}
}

sub process_page_articles_file
{
	select( STDOUT );
	$| = 1;
	my $fin_file = shift();
	my $fin; open( $fin, "<", $fin_file );
	my $fout_file = shift();
	my $fout; open( $fout, ">", $fout_file );
	my $page_on=0; 
	my $page="";
	my $count = 0;

	while( <$fin> )
	{
		trim($_);

		if($_ eq "<page>")
		{
			$page_on = 1;
		}
		elsif($_ eq "</page>")
		{
			$page_on = 0;

			if(not($page =~ m/<redirect.*?\/>/))
			{
				if($page =~ m/.*<title>(.*)<\/title>.*<ns>(.*)<\/ns>.*<id>(.*)<\/id>.*<revision>(.*)<\/revision>.*/)
				{
					my $title = $1;
					my $ns = $2;
					my $id = $3;
					my $revision = $4;

					if($ns eq "0")
					{
						$revision =~ m/<text.*>(.*)<\/text>/;
						my $text = $1;

						if(not($text =~ m/\{\{.*-stub\}\}/))
						{
							$text =~ s/(.*?)==[^=]*==.*/$1/g;    # In case abstract-only features are needed, uncomment this line
							$text =~ s/\[\[Category:.*?\]\]//g;
							print $fout "$id\t$text\n";

							if( $count % 1000==0 )
							{
								print "Progress: $count lines outputted\r";
							}
							$count++;
						}
					}
				}
				else
				{
					die "not possible\n";
				}
			}
			$page = "";
		}
		elsif($page_on == 1)
		{
			$page .= "\t$_";
		}
	}

	close( $fin );
	close( $fout );
}

1;