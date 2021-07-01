import sys
import os

dic = {}

in_file = open("data.txt","r")
for line in in_file:
    items = line.strip().split()
    items = items[0].split(",")
    lbls = []
    for item in items:
        lbls.append(int(item))
    for i in range(len(lbls)-1):
        for j in range(i+1,len(lbls)):
            if dic.has_key((lbls[i],lbls[j])):
                dic[(lbls[i],lbls[j])] = dic[(lbls[i],lbls[j])]+1
            else:
                dic[(lbls[i],lbls[j])] = 1
in_file.close()

more_than_x = [0]*11
for key,value in dic.items():
    for i in range(len(more_than_x)):
        if value>=i:
            more_than_x[i] = more_than_x[i]+1

print more_than_x

single_label = 0
no_lbls_x = [0]*11
for I in range(len(no_lbls_x)):
    in_file = open("data.txt","r")
    for line in in_file:
        items = line.strip().split()
        items = items[0].split(",")
        lbls = []
        for item in items:
            lbls.append(int(item))
        no_lbl = True
        if I==0:
            if len(lbls)==1:
                single_label = single_label+1
        for i in range(len(lbls)-1):
            for j in range(i+1,len(lbls)):
                if dic[(lbls[i],lbls[j])]>=I:
                    no_lbl = False
        if no_lbl:
            no_lbls_x[I] = no_lbls_x[I]+1
    in_file.close()

print no_lbls_x
print single_label
