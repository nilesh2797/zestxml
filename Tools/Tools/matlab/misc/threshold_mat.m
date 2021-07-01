function outM = threshold_mat( M, w_th )
    [X,Y,V] = find( M );
    V( abs(V) < w_th ) = 0;
    outM = sparse( X, Y, V, size(M,1), size(M,2) );
end

