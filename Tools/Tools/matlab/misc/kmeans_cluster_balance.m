function assign_mat = kmeans_cluster_balance( mat, type )

    prm.type = 'NORM_1';
    num_inst = size( mat, 2 );
    half_num_inst = round( num_inst/2 );
    num_ft = size( mat, 1 );
    
    switch type
        case 'cosine'
            [~, mat] = norm_features.fit_transform( mat, prm );
	    norms = [];
            
        case 'euclidean'
            norms = sum( mat.*mat, 1 );
    end
    
	init_assign = initialize_kmeans( mat );

    clust_ctr = mat( :, init_assign );
    clust_ctr = full( clust_ctr );
    old_obj = -1;
    new_obj = 1;
    assign = true(1, num_inst );
    c = 0;
 
    while abs( new_obj - old_obj ) > 1e-7
        c = c+1;
        dist = get_dist( clust_ctr, mat, norms, type );
        diffvals = dist(1,:) - dist(2,:);
        [V,I] = sort( diffvals, 2, 'ascend' );
        assign(:) = false;
        assign( I(1:half_num_inst) ) = true;
        old_obj = new_obj;
        new_obj = full( (sum( dist(1, assign) ) + sum( dist(2, ~assign) ))/num_inst );
        fprintf( '%d: %f\n', c, new_obj );
        clust_ctr = get_clust( mat, assign, type );
    end
    
    assign_mat = [ assign; ~assign];
end

function clust_ctr = get_clust( mat, assign, type )
    prm.type = 'NORM_1';
    clust_ctr = [mat * [assign' ~assign']];
    clust_ctr = full( clust_ctr );

    switch type
        case 'cosine'
            [~, clust_ctr] = norm_features.fit_transform( clust_ctr, prm );
            
        case 'euclidean'
            pn = [ sum(assign) sum(~assign) ];
            pn( pn==0 ) = 1;
            clust_ctr = bsxfun( @rdivide, clust_ctr, pn );
    end
end

function dist = get_dist( clust_ctr, mat, norms, type )
    switch type
        case 'cosine'
            dist = 1-clust_ctr'*mat;
            
        case 'euclidean'
            cnorms = sum( clust_ctr.*clust_ctr, 1 );
            num_inst = size( mat, 2 );
            dist = repmat( cnorms', 1, num_inst ) + repmat( norms, 2, 1 ) - 2*clust_ctr'*mat;
    end
end

function init_assign = initialize_kmeans( mat )
    num_inst = size( mat, 2 );
    init_assign = randsample( num_inst, 2 );
end

function init_assign = initialize_kmeanspp( mat )
    num_inst = size( mat, 2 );

    center1 = randsample( num_inst, 1 );
    sims = mat(:,center1)' * mat;
    dists = 1-sims;
    sqdists = dists.*dists;
    center2 = find( mnrnd(1, sqdists/sum(sqdists) ) );
    init_assign = [ center1 center2 ];
end

