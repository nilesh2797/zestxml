#!/bin/bash

cat $1 | perl -w -ne 'chomp($_); next if($_ =~ m/^\s*tree.*$/ || $_ =~ m/^\s*node.*$/ || $_ =~ m/.*started.*/ || $_ =~ m/.*completed.*/); print "$_\n";' | less -S

