mat = dlmread('triads.0.txt',' ');
mat(:,2) = mat(:,2)+1;
score_mat0 = sparse(mat(:,2),mat(:,1),mat(:,3),8838461,5500000);
save('score_mat0.mat','score_mat0','-v7.3');
