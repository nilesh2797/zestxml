% addpath('/home/deepak/Dropbox/Tools');
[trn_ft_mat,trn_lbl_mat,tst_ft_mat,tst_lbl_mat] = read_train_test('Wikipedia-LSHTC-Tail',0);
cvp = cvpartition(size(trn_lbl_mat,1),'holdout',0.1);

new_trn_lbl_mat = trn_lbl_mat(cvp.test,:);
new_tst_lbl_mat = tst_lbl_mat(cvp.test,:);

trn_ind = sum(new_trn_lbl_mat,1)>0;
tst_ind = sum(new_tst_lbl_mat,1)>0;
new_trn_lbl_mat = new_trn_lbl_mat(:,trn_ind);
new_tst_lbl_mat = new_tst_lbl_mat(:,tst_ind);
new_trn_ft_mat = trn_ft_mat(:,trn_ind);
new_tst_ft_mat = tst_ft_mat(:,tst_ind);
ft_ind = sum(new_trn_ft_mat,2)>0;
new_trn_ft_mat = new_trn_ft_mat(ft_ind,:);
new_tst_ft_mat = new_tst_ft_mat(ft_ind,:);

ft_mat = [new_trn_ft_mat new_tst_ft_mat];
lbl_mat = [new_trn_lbl_mat new_tst_lbl_mat];

write_data('.',ft_mat,lbl_mat);

trn_idx = fopen('trn.0.txt','w');
tst_idx = fopen('tst.0.txt','w');

num_trn = size(new_trn_ft_mat,2);
num_tst = size(new_tst_ft_mat,2);

for i=0:num_trn-1
    fprintf(trn_idx,'%d\n',i);
end

for i=num_trn:num_trn+num_tst-1
    fprintf(tst_idx,'%d\n',i);
end

fclose(trn_idx);
fclose(tst_idx);

1;