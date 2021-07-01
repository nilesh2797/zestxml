#!/bin/bash

url_file=$1
dld_folder=$2

mkdir -p $dld_folder
#wget -P $dld_folder -i $url_file

ctr=0
while read url; do
	ctr=$((ctr+1))
	url=`echo $url | perl -w -ne 'BEGIN{use URI::Escape;} chomp( $_ ); $_ = uri_unescape( $_ ); print "$_";'`
	echo "$ctr	$url" >> url_counter.txt
	echo "curl -o $dld_folder/$ctr.html -O -J -L \"$url\"" >> commands.txt
	curl -o $dld_folder/$ctr.html -O -J -L "$url"
done < $url_file
