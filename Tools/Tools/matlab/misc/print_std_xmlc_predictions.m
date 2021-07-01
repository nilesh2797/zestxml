function print_std_xmlc_predictions( score_mat, X_Y, k, X, Y, freqs, outfile )

	disp( size( score_mat ) );
	
	num_inst = size( score_mat, 2 );
	score_mat = retain_topk( score_mat, k );
	rank_mat = sort_sparse_mat( score_mat );

	fout = fopen( outfile, 'w' );

	fprintf( fout, '###Pattern:\n' );
	fprintf( fout, '###<instance id>:<instance title>\n' );
	fprintf( fout, '###\t<ground_label1 id>:<ground_label1 title>:<ground_label1 freq>\t<ground_label2 id>:<ground_label2 title>:<ground_label2 freq>...\n' );
	fprintf( fout, '###\t<pred_label1 id>:<pred_label1 title>:<pred_label1 freq>:<pred_label1 score>:<pred_label1 correctness>\t<pred_label2 id>:<pred_label2 title>:<pred_label2 freq>:<pred_label2 score>:<pred_label2 correctness>...\n');

	for i=1:num_inst
		scores = score_mat(:,i);
		[sranks, sinds] = sort( rank_mat(:,i), 1, 'descend' );
		labels = X_Y(:,i);
		[linds,~] = find( labels ) ;
		
		fprintf( fout, '\n%d:\t%s\n', i, X{i} );
		for j=1:numel(linds)
			fprintf( fout, '\t%d:%s:%d', full(linds(j)), Y{linds(j)}, full(freqs(linds(j))) );
		end
		fprintf( fout, '\n' );

		for j=k:-1:1
			fprintf( fout, '\t%d:%s:%d:%f:%d', full(sinds(j)), Y{sinds(j)}, full(freqs(sinds(j))), full(scores(sinds(j))), full(labels(sinds(j))) );
		end
		fprintf( fout, '\n' );
	end

	fclose( fout );

end
