# ./run_eurlex.sh xhtp_approx -bs_count 40;
# ./run_eurlex.sh xhtp_fine_tune;
# ./run_eurlex.sh tune;

./run_eurlex.sh xhtp_approx -bs_count 120;
./run_eurlex.sh xhtp_fine_tune;
./run_eurlex.sh tune;

./run_eurlex.sh xhtp_approx -bs_count 160;
./run_eurlex.sh xhtp_fine_tune;
./run_eurlex.sh tune;

./run_eurlex.sh xhtp_approx;
./run_eurlex.sh xhtp_fine_tune -bilinear_classifier_cost 0.2;
./run_eurlex.sh tune;

./run_eurlex.sh xhtp_fine_tune -bilinear_classifier_cost 0.5;
./run_eurlex.sh tune;

./run_eurlex.sh xhtp_fine_tune -bilinear_classifier_cost 1;
./run_eurlex.sh tune;

./run_eurlex.sh xhtp_fine_tune -bilinear_classifier_cost 2;
./run_eurlex.sh tune;

./run_eurlex.sh xhtp_fine_tune -bilinear_classifier_cost 5;
./run_eurlex.sh tune;

./run_eurlex.sh xhtp_fine_tune -bilinear_classifier_cost 10;
./run_eurlex.sh tune;

./run_eurlex.sh xhtp_fine_tune -shortyK 20 -bilinear_shortyK 20;
./run_eurlex.sh tune -shortyK 20 -bilinear_shortyK 20;

./run_eurlex.sh xhtp_fine_tune -shortyK 50 -bilinear_shortyK 50;
./run_eurlex.sh tune -shortyK 50 -bilinear_shortyK 50;

./run_eurlex.sh xhtp_fine_tune -shortyK 80 -bilinear_shortyK 80;
./run_eurlex.sh tune -shortyK 80 -bilinear_shortyK 80;

./run_eurlex.sh xhtp_fine_tune -shortyK 120 -bilinear_shortyK 120;
./run_eurlex.sh tune -shortyK 120 -bilinear_shortyK 120;

./run_eurlex.sh xhtp_fine_tune -shortyK 150 -bilinear_shortyK 150;
./run_eurlex.sh tune -shortyK 150 -bilinear_shortyK 150;

./run_eurlex.sh xhtp_fine_tune -shortyK 200 -bilinear_shortyK 200;
./run_eurlex.sh tune -shortyK 200 -bilinear_shortyK 200;