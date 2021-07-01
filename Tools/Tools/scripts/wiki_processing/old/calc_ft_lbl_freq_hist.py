import sys
import os

ft_file = open("features.txt","r")
ft_hist = {}
for line in ft_file:
    items = line.strip().split(",")
    freq = int(items[1])
    if ft_hist.has_key(freq):
        ft_hist[freq] = ft_hist[freq]+1
    else:
        ft_hist[freq] = 1
ft_file.close()

ft_hist_file = open("ft_hist.txt","w")
ks = ft_hist.keys()
ks.sort()

for key in ks:
    print >>ft_hist_file, "%d:%d" %(key,ft_hist[key])
ft_hist_file.close()

lbl_file = open("labels.txt","r")
lbl_hist = {}
for line in lbl_file:
    items = line.strip().split(",")
    freq = int(items[1])
    if lbl_hist.has_key(freq):
        lbl_hist[freq] = lbl_hist[freq]+1
    else:
        lbl_hist[freq] = 1
lbl_file.close()

lbl_hist_file = open("lbl_hist.txt","w")
ks = lbl_hist.keys()
ks.sort()

for key in ks:
    print >>lbl_hist_file, "%d:%d" %(key,lbl_hist[key])
lbl_hist_file.close()
