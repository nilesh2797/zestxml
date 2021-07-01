#!/bin/bash

cat BingAdsGIData | perl -w -ne 'chomp($_); my @items = split( "\t", $_ ); next if( $items[3]<5 ); print "$_\n";' > BingAdsGIData.filtered
cat BingAdsGIData.filtered | perl uniq_fields.pl 1 "\t" > Y.txt
cat BingAdsGIData.filtered | perl uniq_fields.pl 2 "\t" > X.txt.1
cat UrlTitleTokensFile | perl uniq_fields.pl 1 "\t" > X.txt.2
perl intersection.pl X.txt.1 X.txt.2 > trn_X.txt
perl process_features.pl X.txt UrlTitleTokensFile tokens.txt
cat tokens.txt | perl clean_text.pl > tokens.txt.cleaned
cat tokens.txt.cleaned | perl words_2_spmat.pl " " Xf.txt 0 > trn_X_Xf.txt
perl process_labels.pl X.txt Y.txt BingAdsGIData.filtered trn_X_Y_cl.txt trn_X_Y_im.txt trn_X_Y_ctr.txt