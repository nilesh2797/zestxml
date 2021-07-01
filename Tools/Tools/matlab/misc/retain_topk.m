function outmat = retain_topk( mat, k )
	rank_mat = sort_sparse_mat( mat );
	rank_mat( rank_mat>k ) = 0;
	rank_mat = spones( rank_mat );
	outmat = mat .* rank_mat;
end
