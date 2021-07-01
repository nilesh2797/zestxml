function subsample_dataset( dset )

    rng( 1 );

    [X_Xf, X_Y, Y_Yf] = load_dataset( dset );
    frac = 0.2;
    
    num_X = size( X_Xf, 2 );
    act_X = find( rand( num_X, 1 ) <= frac );
    act_Xf = find( sum( X_Xf( :, act_X )>0, 2 ) >= 2 );
    act_Y = find( sum( X_Y( :, act_X ), 2 ) >= 2 );
    act_Yf = find( sum( Y_Yf( :, act_Y )>0, 2 ) >= 2 );

    X_Xf = X_Xf( act_Xf, act_X );
    Y_Yf = Y_Yf( act_Yf, act_Y );
    X_Y = X_Y( act_Y, act_X );
    split = split_data( X_Y, [], [], 1 );
    
    save( 'X_Xf.mat', 'X_Xf', '-v7.3' );
    save( 'Y_Yf.mat', 'Y_Yf', '-v7.3' );
    save( 'X_Y.mat', 'X_Y', '-v7.3' );
    csvwrite( 'split.0.txt', split );
end