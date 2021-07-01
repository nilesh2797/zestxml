function [ clust_ctr, assign_mat ] = kmeans_cluster( mat, num_clust, type )

    if strcmp( type, 'cosine' )
        prm.type = 'NORM_1';
        [~, mat] = norm_features.fit_transform( mat, prm );
    end

    num_inst = size( mat, 2 );
    num_ft = size( mat, 1 );
    init_assign = randsample( num_inst, num_clust );
    old_clust_ctr = zeros( num_ft, num_clust );
    clust_ctr = mat( :, init_assign );
    old_obj = -1;
    new_obj = 1;
    
    while abs( new_obj - old_obj ) > 1e-6 
        if strcmp( type, 'cosine' )
            old_clust_ctr = clust_ctr;
            dist = clust_ctr'*mat;
            [~, assign] = max( dist, [], 1 );
            assign_mat = sparse( assign, 1:num_inst, ones(1,num_inst), num_clust, num_inst );
            clust_ctr = mat*assign_mat';
            [~, clust_ctr] = norm_features.fit_transform( clust_ctr, prm );
            
            old_obj = new_obj;
            new_obj = full( sum( sum( clust_ctr .* (mat*assign_mat') ) ) ) / num_inst;
            fprintf( 'obj val: %f\n', new_obj );
        end
    end
end


