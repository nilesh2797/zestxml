#!/bin/bash

shopt -s globstar

fnames=( X_Xf X_Y Y_Yf X_Y_click X_Y_impression )

for var in "${fnames[@]}"
do
	echo "$var"
	for fil in **/$var.txt
	do
		echo $fil
		outfil=`echo $fil | sed "s/.txt/.npz/g"`
		echo $outfil
python <<PY
from libimport import *
from utils import *
mat = read_text_mat( '$fil' )
save_npz( '$outfil', mat )
PY
	done
done
