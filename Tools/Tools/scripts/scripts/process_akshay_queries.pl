#!/bin/bash

cat AdcNormQueryMetricUS20180918.tsv | perl -w -ne 'chomp($_); my @items = split( "\t", $_ ); print "$items[0]\t$items[2]\n";' | perl -w -ne 'BEGIN{ my %dict = (); } chomp($_); my @items = split( "\t", $_ ); $dict{ $items[0] } = $items[1]; END{ print "$_\t$dict{$_}\n" for( sort {$dict{$b} <=> $dict{$a}} keys(%dict) );  }' > queriesAkshayRevenue.txt

perl normalize.pl queriesAkshayRevenue.txt normalizedQueriesAkshayRevenue.txt

