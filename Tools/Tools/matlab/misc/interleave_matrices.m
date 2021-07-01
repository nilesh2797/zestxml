function outmat = interleave_matrices( mat1, mat2, row_or_col )
	% row_or_col==1 => row interleave, row_or_col=2 => column interleave
	assert( size(mat1,1)==size(mat2,1) && size(mat1,2)==size(mat2,2) );
	if row_or_col==1
		outmat = reshape([mat1(:) mat2(:)]',2*size(mat1,1), []);
	elseif row_or_col==2
		mat1 = mat1';
		mat2 = mat2';
		outmat = reshape([mat1(:) mat2(:)]',2*size(mat1,1), [])';
	end
end
