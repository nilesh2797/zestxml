# In case abstract-only features are needed, uncomment relevant line below

wiki_cat_subcat="tmp2.txt"
wiki_cat_page="tmp3.txt"
wiki_id_page_map="tmp4.txt"
wiki_pages_articles="tmp5.txt"
data_file="data.txt"
hierarchy_file="hierarchy.txt"
title_file="titles.txt"
tmp_file="tmp.txt"
tmp1_file="tmp_1.txt"
label_file="labels.txt"
feature_file="features.txt"

accum_rows='BEGIN {use Text::Trim; $curr="-1"; $str="";} 
trim($_); my @fields = split("\t",$_,2); 
if($curr eq "-1")
{
	$curr = $fields[0];
	$str = $fields[1];
}
elsif($curr eq $fields[0])
{
	$str .= ",".$fields[1];
}
else
{
	print $curr."\t".$str."\n";
	$curr = $fields[0];
	$str = $fields[1];
}
'

merge_files='BEGIN {open $f1, "<", $ARGV[0] or die $!; open $f2, "<", $ARGV[1] or die $!;}
sub read_file
{
	my $file = shift;
	my $line = <$file>;
	$line or return;
	chomp($line);
	my @items = split("\t",$line,2);
	return @items;
}

my $first = "-1";
my $second = "-2";
my $first_text = "";
my $second_text = "";
my @items;

while(1)
{
	if($first < $second)
	{
		(@items = read_file($f1)) or last;
		($first,$first_text) = @items;
	}
	elsif($first > $second)
	{
		(@items = read_file($f2)) or last;
		($second,$second_text) = @items;
	}
	else
	{
		print $first."\t".$first_text."\t".$second_text."\n";
		(@items = read_file($f1)) or last;
		($first,$first_text) = @items;
		(@items = read_file($f2)) or last;
		($second,$second_text) = @items;
	}
}
'

sed "s/\\\'/@@/g" enwiki-20141208-categorylinks.sql | 
perl -ne 'BEGIN {open (PFILE,"> tmp3.txt"); open (CFILE,"> tmp2.txt");} while( /\(([0-9]*),'\''([^'\'']*)'\'','\''[^'\'']*'\'','\''[^'\'']*'\'','\''[^'\'']*'\'','\''[^'\'']*'\'','\''(page|subcat)'\''\)/g )
{
	my $str = $2;
	my $one = $1;
	my $three = $3;
	
	$str =~ s/@@/\\'\''/g;

	if($three eq "page")
	{
		print PFILE $one."\t'\''".$str."'\''\n";
	}
	else
	{
		print CFILE $one."\t'\''".$str."'\''\n";
	}
}
END {close(PFILE); close(CFILE);}' & 


sed "s/\\\'/@@/g" enwiki-20141208-page.sql | 
perl -ne 'while( /\(([0-9]*),([0-9]*),'\''([^'\'']*)'\'','\''[^'\'']*'\'',([0-9]*),([0-9]*),([0-9]*),[^()]*\)/g )
{
	if(($2 eq "0" or $2 eq "14") and $6 eq "0")
	{
		print $1."\t".$3."\n";
	}
}' | sed "s/@@/\\\'/g" > $wiki_id_page_map &

echo "waiting..."
wait

cat enwiki-20141208-pages-articles-multistream.xml |
perl -ne 'BEGIN {
use Text::Trim;
use Lingua::StopWords qw(getStopWords);
use Lingua::Stem::Snowball;
$stemmer = Lingua::Stem::Snowball->new( lang => "en" );
$stopwords = getStopWords("en");
%stopword_hash = %$stopwords;
$page_on=0; $page="";
}
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
					# $text =~ s/(.*?)==[^=]*==.*/$1/g;    # In case abstract-only features are needed, uncomment this line
					$text =~ s/\[\[Category:.*?\]\]//g;
					$text =~ s/\W+/ /g;
					my @words = split(/ +/,$text);
					my %word_dict = ();

					print $id."\t";

					@words = grep {! exists $stopword_hash{$_}} @words;

					$stemmer->stem_in_place(\@words);

					for my $i (0 .. $#words)
					{
						my $word = lc $words[$i];
						if(exists($word_dict{$word}))
						{
							$word_dict{$word}++;
						}
						else
						{
							$word_dict{$word} = 1;
						}
						#print $word."\t".$word_dict{$dict}."\n";
					}

					my $i = 0;
					while ( ($key, $value) = each %word_dict )
					{
						if($i==0)
						{
							print $key.":".$value;
						}
						else
						{
							print ",".$key.":".$value;
						}
						$i++;
					}
					print "\n";
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
	$page .= "\t" . $_;
}
' > $wiki_pages_articles &
echo "waiting2..."
wait

(perl -e "$merge_files" <(cat $wiki_cat_page | perl -ne "$accum_rows") <(cat $wiki_pages_articles)) > $data_file


(perl -e "$merge_files" <(cat $wiki_id_page_map) <(cat $wiki_cat_subcat | perl -ne "$accum_rows")) | \
(awk -F "	" '{print "'\''" $2 "'\''\t" $3 }') > $hierarchy_file


perl -e "$merge_files" <(cat $wiki_id_page_map) <(cat $data_file) | \
perl -ne 'BEGIN {open($title_file,">",$ARGV[0]); open($tmp_file,">",$ARGV[1]); @ARGV=();}
chomp($_);
my @fields = split("\t",$_,4);
print $title_file $fields[1]."\n";
print $tmp_file $fields[2]."\t".$fields[3]."\n";
#print $fields[0]."--".$fields[1]."--".$fields[2]."\n";
END {close($title_file); close($tmp_file);}' $title_file $tmp_file
mv $tmp_file $data_file


min_lbl_freq="2"
min_ft_freq="5"

cat $data_file | \
perl -ne 'BEGIN {$ctr=0;open($lbl_file,">",$ARGV[0]); open($ft_file,">",$ARGV[1]); $min_lbl_freq=$ARGV[2]; $min_ft_freq=$ARGV[3]; @ARGV=(); %lbl_freq={}; %ft_freq={};}
chomp($_);
my @fields = split("\t",$_);
$fields[0] = substr($fields[0],1,length($fields[0])-2);
$fields[0] =~ s/\\'\''/@@/g;
my @labels = split("'\'','\''",$fields[0]);
$lbl_freq{$_}++ for(@labels);
my @features = split(",",$fields[1]);
map {s/(.*):.*/$1/;} @features;
$ft_freq{$_}++ for (@features);
print "$ctr\n" if $ctr%100==0; $ctr++;
END 
{
my @lbl_keys = grep {$lbl_freq{$_}>=$min_lbl_freq} (keys %lbl_freq);
map {s/@@/\\'\''/g} @lbl_keys;
print $lbl_file "'\''$_'\''\n" for (sort(@lbl_keys));
my @ft_keys = grep {$ft_freq{$_}>=$min_ft_freq} (keys %ft_freq);
print $ft_file "$_\n" for (sort(@ft_keys));
close($lbl_file); close($ft_file);
}
' $label_file $feature_file $min_lbl_freq $min_ft_freq


perl -w -e 'BEGIN{ open($hierarchy_file,"<",$ARGV[0]); open($label_file,"<",$ARGV[1]); open($new_label_file,">",$ARGV[2]); @ARGV=()}
my %hidden_cats=();
while(<$hierarchy_file>)
{
	chomp($_);
	my @fields = split("\t",$_,2);
	$fields[1] =~ s/\\'\''/@@/g;
	$fields[1] = substr($fields[1],1,length($fields[1])-2);
	my @labels = split("'\'','\''",$fields[1]);
	@labels = grep {/^Hidden_categories$/} @labels;
	if(scalar @labels)
	{
		$hidden_cats{$fields[0]} = 1;	
	}
}

my @labels = <$label_file>;
map {chomp} @labels;
@labels = grep {not(exists($hidden_cats{$_}))} @labels;
@labels = grep {!/^'\''(Article_sections|Articles_|Pages_|Wikipedia_|CS1_|Use_).*/} @labels;
@labels = grep {!/^'\''All_.*(articles|pages).*/;} @labels;
@labels = grep {!/.*disambiguation.*/i;} @labels;
print $new_label_file join("\n",@labels)."\n";

END{close($hierarchy_file); close($label_file); close($new_label_file);}

' $hierarchy_file $label_file $tmp_file
mv $tmp_file $label_file


cat $data_file | \
perl -w -ne 'BEGIN {
my $title_file;open($title_file,"<",$ARGV[0]);@titles=<$title_file>;map {chomp} @titles;close($title_file);
my $lbl_file;open($lbl_file,"<",$ARGV[1]);my @labels=<$lbl_file>;map {chomp} @labels;close($lbl_file);
%lbl_id = (); my $ctr=0; $lbl_id{$_}=$ctr++ for(@labels);
my $ft_file;open($ft_file,"<",$ARGV[2]);my @features=<$ft_file>;map {chomp} @features;close($ft_file);
%ft_id = (); $ctr=0; $ft_id{$_}=$ctr++ for(@features);
open($new_title_file,">",$ARGV[3]);open($new_data_file,">",$ARGV[4]);
@ARGV=();
$line_no=0;
}
chomp($_);
my @fields = split("\t",$_);
$fields[0] = substr($fields[0],1,length($fields[0])-2);
$fields[0] =~ s/\\'\''/@@/g;
my @labels = split("'\'','\''",$fields[0]);
map {s/@@/\\'\''/g; s/(.*)/'\''$1'\''/;} @labels;
@labels = sort {$a <=> $b} (map {$lbl_id{$_};} (grep {exists($lbl_id{$_})} @labels));
my @features = map {m/(.*):(.*)/;[$1,$2];} split(",",$fields[1]);
@features =  sort {$a->[0] <=> $b->[0]} (map {[$ft_id{$_->[0]},$_->[1]]} (grep {exists($ft_id{$_->[0]})} @features));

if ((scalar @labels == 0) || (scalar @features == 0))
{
	$line_no++; next;
}

print $new_title_file "$titles[$line_no]\n";
print $new_data_file join(",",@labels)." ".join(" ",map {"$_->[0]:$_->[1]"} @features)."\n";
$line_no++;

END {close($new_title_file);close($new_data_file);}
' $title_file $label_file $feature_file $tmp_file $tmp1_file



mv $tmp_file $title_file
mv $tmp1_file $data_file

#rm tmp*.txt


cat $hierarchy_file | perl -w -ne 'BEGIN { %hier_hash=();open($new_hierarchy_file,">",$ARGV[0]); @ARGV=();}
chomp($_);
my @fields = split("\t",$_,2);
$fields[1] = substr($fields[1],1,length($fields[1])-2);
$fields[1] =~ s/\\'\''/@@/g;
my @labels = split("'\'','\''",$fields[1]);

for $label(@labels)
{
	$label =~ s/@@/\\'\''/g;
	$label = "'\''".$label."'\''";

	if(exists($hier_hash{$label}))
	{	
		$hier_hash{$label} .= ",".$fields[0];
	}
	else
	{
		$hier_hash{$label} = $fields[0];
	}
}

END {
for $key(sort(keys %hier_hash))
{
	print $new_hierarchy_file $key."\t".$hier_hash{$key}."\n";
}

close($new_hierarchy_file);
}
' $tmp_file

mv $tmp_file $hierarchy_file


