function inc_tst_X_Y = get_shilpa_inc_mat( varargin )
	% expects unweighted label matrix
	% outputs subsampled unweighted label matrix

	tst_X_Y = varargin{1};
	frac = varargin{2};

	if nargin==3
		seed = varargin{3};
	else
		seed = 1;
	end

    rng( seed );
 
    num_tst = size(tst_X_Y,2);
    num_lbl = size(tst_X_Y,1);
    [X,Y,V] = find(tst_X_Y);
   
    V = rand(numel(V),1);
    rd_mat = sparse(X,Y,V,num_lbl,num_tst);
    rank_mat = sort_sparse_mat(rd_mat);
    
    ths = floor(frac*sum(tst_X_Y,1));
    th_mat = bsxfun(@times, tst_X_Y, ths);
    
    rank_mat(rank_mat>th_mat) = 0;
    inc_tst_X_Y = spones(rank_mat);
end
