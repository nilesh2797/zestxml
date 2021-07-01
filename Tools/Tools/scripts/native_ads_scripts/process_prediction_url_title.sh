#!/bin/bash

##perl clean_prediction_url_title.pl dataDumpWithKWwithBlankLinesTwoDays.txt tst_X.txt tst_words.txt
#cat tst_words.txt | perl clean_text.pl > tst_words.txt.cleaned
#cat tst_words.txt.cleaned | perl words_2_spmat.pl " " Xf.txt 1 > tst_X_Xf.txt

cat tst_words_next8M.txt | perl clean_text.pl > tst_words_next8M.txt.cleaned
cat tst_words_next8M.txt.cleaned | perl words_2_spmat.pl " " Xf.txt 1 > tst_X_Xf_next8M.txt