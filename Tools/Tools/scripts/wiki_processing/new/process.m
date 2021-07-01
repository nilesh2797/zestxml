clear all;
tools_dir = '/yashoteja/Algorithms/Code/Tools';
cd '.';
addpath(tools_dir);
[trn_ft_mat,trn_lbl_mat,tst_ft_mat,tst_lbl_mat] = read_train_test('.',0);
ft_mat = [trn_ft_mat tst_ft_mat];
lbl_mat = [trn_lbl_mat tst_lbl_mat];

num_inst = size(ft_mat,2);
num_lbl = size(lbl_mat,1);
num_ft = size(ft_mat,1);

valid_lbl_ind = 1:num_lbl;
% sum(lbl_mat,2)<=5 & sum(lbl_mat,2)>1;
% frac = 0.2;
% cvp = cvpartition(sum(valid_lbl_ind),'holdout',frac);
% valid_lbl_ind(valid_lbl_ind) = cvp.test;


sampled_lbl_mat = lbl_mat(valid_lbl_ind,:);
% sum_sampled_lbl_mat = sum(sampled_lbl_mat,1);
valid_inst_ind = 1:num_inst;
filtered_lbl_mat = sampled_lbl_mat(:,valid_inst_ind);

filtered_ft_mat = ft_mat(:,valid_inst_ind);
valid_ft_ind = 1:num_ft;
filtered_ft_mat = filtered_ft_mat(valid_ft_ind,:);

% instances = importdata('instances.txt','\t');
% instances = instances(valid_inst_ind,:);
% instances(:,1) = 1:numel(valid_inst_ind);

num_inst = size(filtered_ft_mat,2);
num_ft = size(filtered_ft_mat,1);
num_lbl = size(filtered_lbl_mat,1);

trn_or_tst = zeros(num_inst,1);

filtered_lbl_mat = filtered_lbl_mat';
for i=1:num_lbl
    if mod(i,100)==0
	i
    end
    insts = find(filtered_lbl_mat(:,i));
    if numel(insts)==0
        continue;
    end
    if sum(trn_or_tst(insts)==1)==0
        rand_ind = randi(numel(insts));
        trn_or_tst(insts(rand_ind)) = 1;
        insts(rand_ind) = [];
    end
    if numel(insts)>0 && sum(trn_or_tst(insts)==2)==0
        rand_ind = randi(numel(insts));
        trn_or_tst(insts(rand_ind)) = 2;
        insts(rand_ind) = [];
    end
end

for i=1:num_inst
%     i
    if trn_or_tst(i)==0
        if rand<0.8
            trn_or_tst(i) = 1;
        else
            trn_or_tst(i) = 2;
        end
    end
end

trn_file = fopen('trn.0.txt','w');
tst_file = fopen('tst.0.txt','w');

for i=1:num_inst
    if trn_or_tst(i)==1
        fprintf(trn_file,'%d\n',i-1);
    elseif trn_or_tst(i)==2
        fprintf(tst_file,'%d\n',i-1);
    end
end

fclose(trn_file);
fclose(tst_file);

1;
