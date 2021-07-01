function outM = truncate_mat( M, nnzfrac )
    if nnz( M )<1000
        outM = M;
        return;
    end

    T = round( numel(M) * nnzfrac );
    [X,Y,V] = find( M );

	if T>= numel( V )
		outM = M;
		return;
	end

    [sortV, ~] = sort( abs(V), 1, 'descend' );
    th = sortV( T );
    V( abs(V) < th ) = 0;
    outM = sparse( X, Y, V, size(M,1), size(M,2) );
end

