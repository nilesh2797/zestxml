function postprocess( X_Xf_file, X_Y_file, X_file, Xf_file, Y_file, trn_X_Xf_file, trn_X_Y_file, tst_X_Xf_file, tst_X_Y_file, trn_X_file, tst_X_file, new_Xf_file, new_Y_file )

	X_Xf = read_text_mat( X_Xf_file );
	X_Y = read_text_mat( X_Y_file );
	X = read_string_file( X_file );
	Xf = read_string_file( Xf_file );
	Y = read_string_file( Y_file );

	valid_Xf = sum( X_Xf>0, 2 )>=5;
	valid_Y = sum( X_Y>0, 2 )>=2;
	X_Xf = X_Xf( valid_Xf, : );
	X_Y = X_Y( valid_Y, : );
	valid_X = sum( X_Y>0, 1 )>=1;
	X_Xf = X_Xf( :, valid_X );
	X_Y = X_Y( :, valid_X );
	X = X{ valid_X };
	Xf = Xf{ valid_Xf };
	Y = Y {valid_Y };
	
	prm.type = 'TF_IDF';
	[tfifd_model, X_Xf] = norm_features.fit_transform( X_Xf, prm );
	save( 'tfidf_model.mat', 'tdidf_model', '-v7.3' );

	split = split_data( X_Y, [], 0, 1 );
	split( end-20:end ) = 1;

	trn_X = X{ split==0 };
	tst_X = X{ split==1 };
	trn_X_Xf = X_Xf( :, split==0 );
	tst_X_Xf = X_Xf( :, split==1 );
	trn_X_Y = X_Y( :, split==0 );
	tst_X_Y = X_Y( :, split==1 );

	write_text_mat( trn_X_Xf, trn_X_Xf_file );
	write_text_mat( tst_X_Xf, tst_X_Xf_file );
	write_text_mat( trn_X_Y, trn_X_Y_file );
	write_text_mat( tst_X_Y, tst_X_Y_file );
	write_string_file( trn_X, trn_X_file );
	write_string_file( tst_X, tst_X_file );
	write_string_file( Xf, new_Xf_file );
	write_string_file( Y, new_Y_file );
end
