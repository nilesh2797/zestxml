function outmat = pow_mat( inmat, A )
	[X,Y,V] = find( inmat );
	V = V.^A;
	outmat = sparse( X, Y, V, size(inmat,1), size(inmat,2) );
end
