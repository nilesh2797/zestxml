import sys
import os
import pdb
import random

ft_tags = []
lbl_tags = []
ft_mat = []
lbl_mat = []

data_on = False
in_file = open("bookmarks.arff","r")

ctr = 0
for line in in_file:
    line = line.strip()
    if line=="@data":
        data_on = True
        continue

    if data_on:
        line = line[1:-1]
        ft_mat.append([])
        lbl_mat.append([])
        id_vals = line.split(",")
        for item in id_vals:
            id_val = item.split(" ")
            Id = int(id_val[0])
            val = float(id_val[1])
            if Id<len(ft_tags):
                ft_mat[ctr].append((Id,val))
            else:
                lbl_mat[ctr].append(Id-len(ft_tags))
        #print ft_mat[ctr]
        #pdb.set_trace()
        #print lbl_mat[ctr]
        #pdb.set_trace()
        ctr = ctr+1
    else:
        items = line.split(" ")
        if items[0]=="@attribute":
            if items[1][:3]=="TAG":
                lbl_tags.append(items[1][4:])
            else:
                ft_tags.append(items[1])
in_file.close()

min_ft = sys.maxint
max_ft = -1

for ft_vec in ft_mat:
    for id_val in ft_vec:
        if id_val[0]<min_ft:
            min_ft = id_val[0]
        if id_val[0]>max_ft:
            max_ft = id_val[0]

print min_ft,max_ft,len(ft_tags)
    
min_lbl = sys.maxint
max_lbl = -1

for lbl_vec in lbl_mat:
    for Id in lbl_vec:
        if Id<min_lbl:
            min_lbl = Id
        if Id>max_lbl:
            max_lbl = Id

print min_lbl,max_lbl,len(lbl_tags)

out_file = open("ft_tags.txt","w")
for tag in ft_tags:
    print >>out_file, tag
out_file.close()

out_file = open("lbl_tags.txt","w")
for tag in lbl_tags:
    print >>out_file, tag
out_file.close()

num_inst = len(ft_mat)
out_file = open("data.txt","w")
for i in range(num_inst):
    ft_vec = ft_mat[i]
    lbl_vec = lbl_mat[i]
    if len(lbl_vec)==0:
        print "zero"
        pdb.set_trace()
    print >>out_file, ",".join(map(lambda x:str(x),lbl_vec)),
    print >>out_file, " ".join(map(lambda x:"%d:%.2f" %(x[0],x[1]),ft_vec))
out_file.close()

num_trn = int(0.8*num_inst)
num_tst = num_inst-num_trn
for i in range(10):
    inds = []
    for j in range(num_inst):
        inds.append(j)
    random.shuffle(inds)
    trn_file = open("trn.%d.txt" %(i),"w")
    tst_file = open("tst.%d.txt" %(i),"w")

    trn_inds = sorted(inds[:num_trn])
    tst_inds = sorted(inds[num_trn:])

    #print len(trn_inds),num_trn
    #print len(tst_inds),num_tst

    for j in range(num_trn):
        print >>trn_file, trn_inds[j]

    for j in range(num_tst):
        print >>tst_file, tst_inds[j]




    trn_file.close()
    tst_file.close()
