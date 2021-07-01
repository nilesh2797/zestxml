function convert_my_format_to_libsvm( dset, split_no, out_trn_file, out_tst_file )
    global EXP_DIR;

    [X_Xf, X_Y] = load_dataset( dset );
    X_Xf_file = tempname;
    X_Y_file = tempname;
    split_file = fullfile( EXP_DIR, 'Datasets', dset, sprintf('split.%d.txt', split_no) );

    write_text_mat( X_Xf, X_Xf_file );
    write_text_mat( X_Y, X_Y_file );

    cmd = sprintf( 'perl convert_my_format_to_libsvm.pl %s %s %s %s %s', X_Xf_file, X_Y_file, split_file, out_trn_file, out_tst_file );
    disp( cmd );
    system( cmd );
end