function s = sparse_std( mat )
	n = size( mat, 2 );
	avg = sum( mat, 2 )/n;
	s = sum( mat.*mat, 2 )/n - avg.*avg; 
end
