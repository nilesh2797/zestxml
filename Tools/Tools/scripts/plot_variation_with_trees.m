output_folder_name = '/media/yashoteja/Storage/Learning/Research/Algorithms/Results/fastXML';
datasets = {'BibTeX','Delicious','MediaMill','EUR-Lex','Rcv1-Modified'};
normal = [];
greedy = [];
backward_greedy = [];

for i=1:5
    cwd = pwd;
    cd(sprintf('%s/%s',output_folder_name,datasets{i}));
    P1_vec = dlmread('p1_normal_order.txt','\t');
    normal = [normal P1_vec(:,2)];
    P1_vec = dlmread('p1_greedy_order.txt','\t');
    greedy = [greedy P1_vec(:,2)];
    P1_vec = dlmread('p1_backward_greedy_order.txt','\t');
    backward_greedy = [backward_greedy P1_vec(:,2)];
    cd(cwd);
end

values = normal;
figure;
plot([1:50]',values(:,1),[1:50]',values(:,2),[1:50]',values(:,3),[1:50]',values(:,4),[1:50]',values(:,5));
leg = legend('BibTeX','Delicious','MediaMill','EUR-Lex','RCV1-Modified');
set(leg,'Location','SouthEast')
xlabel('No. of Trees');
ylabel('Precision at 1');
title('Normal order of trees');

values = greedy;
figure;
plot([1:50]',values(:,1),[1:50]',values(:,2),[1:50]',values(:,3),[1:50]',values(:,4),[1:50]',values(:,5));
leg = legend('BibTeX','Delicious','MediaMill','EUR-Lex','RCV1-Modified');
set(leg,'Location','SouthEast')
xlabel('No. of Trees');
ylabel('Precision at 1');
title('Greedy selection of trees');

values = backward_greedy;
figure;
plot([1:50]',values(:,1),[1:50]',values(:,2),[1:50]',values(:,3),[1:50]',values(:,4),[1:50]',values(:,5));
leg = legend('BibTeX','Delicious','MediaMill','EUR-Lex','RCV1-Modified');
set(leg,'Location','SouthEast')
xlabel('No. of Trees');
ylabel('Precision at 1');
title('Backward greedy selection of trees');