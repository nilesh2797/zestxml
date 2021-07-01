#include "metrics.h"
using namespace std;

VecF precision_k( SMatF* X_Y, SMatF* score_rank_mat, int K )
{
    int num_X = X_Y->nc;
    int num_Y = X_Y->nr;

    VecB labels( num_Y, false );
    VecF precision( K, 0 );

    for( int i=0; i<num_X; i++ )
    {
        for( int j=0; j<X_Y->size[i]; j++ )
            labels[ X_Y->data[i][j].first ] = true;

        for( int j=0; j<score_rank_mat->size[i]; j++ )
        {
            int id = score_rank_mat->data[i][j].first;
            int rank = (int)score_rank_mat->data[i][j].second;

            if( rank<=K && labels[id] )
            {
                for( int k=rank-1; k<K; k++ )
                    precision[k]++;
            }
        }

        for( int j=0; j<X_Y->size[i]; j++ )
            labels[ X_Y->data[i][j].first ] = false;
    }
    for( int k=0; k<K; k++ )
        precision[k] /= ((k+1)*num_X);

    return precision;
}

VecF nDCG_k( SMatF* X_Y, SMatF* score_rank_mat, int K )
{
    int num_X = X_Y->nc;
    int num_Y = X_Y->nr;

    VecF coeffs( num_Y + 1 );
    VecF cum_coeffs( num_Y + 1 );
    coeffs[0] = 0;
    cum_coeffs[0] = 0;

    for( int i=1; i<=num_Y; i++ )
    {
        coeffs[i] = (float) (1.0/log( i+1 ));
        cum_coeffs[i] = cum_coeffs[i-1] + coeffs[i];
    }
    cum_coeffs[0] = 1;

    VecB labels( num_Y, false );
    VecF nDCG( K, 0 );

    for( int i=0; i<num_X; i++ )
    {
        for( int j=0; j<X_Y->size[i]; j++ )
            labels[ X_Y->data[i][j].first ] = true;

        for( int j=0; j<score_rank_mat->size[i]; j++ )
        {
            int id = score_rank_mat->data[i][j].first;
            int rank = (int)score_rank_mat->data[i][j].second;

            if( rank<=K && labels[id] )
            {
                for( int k=rank-1; k<K; k++ )
                {
                    //nDCG[k] += coeffs[rank] / cum_coeffs[ X_Y->size[i] ];
                    nDCG[k] += coeffs[rank] / cum_coeffs[ min( k+1, X_Y->size[i] ) ];
                }
            }
        }

        for( int j=0; j<X_Y->size[i]; j++ )
            labels[ X_Y->data[i][j].first ] = false;
    }
    for( int k=0; k<K; k++ )
    {
        nDCG[k] /= num_X;
    }

    return nDCG;
}

VecF psprecision_k_helper( SMatF* X_Y, SMatF* rank_mat, int K )
{
    int num_X = X_Y->nc;
    int num_Y = X_Y->nr;

    VecF labels( num_Y, 0 );
    VecF psprecision( K, 0 );

    for( int i=0; i<num_X; i++ )
    {
        for( int j=0; j<X_Y->size[i]; j++ )
        {
            int id = X_Y->data[i][j].first;
            float val = X_Y->data[i][j].second; 
            labels[ id ] = val;
        }

        for( int j=0; j<rank_mat->size[i]; j++ )
        {
            int id = rank_mat->data[i][j].first;
            int rank = (int)rank_mat->data[i][j].second;

            if( rank<=K && labels[id] )
            {
                for( int k=rank-1; k<K; k++ )
                    psprecision[k] += labels[id];
            }
        }

        for( int j=0; j<X_Y->size[i]; j++ )
            labels[ X_Y->data[i][j].first ] = 0;
    }

    return psprecision;
}

VecF psprecision_k( SMatF* X_Y, SMatF* score_rank_mat, int K )
{
    SMatF* true_rank_mat = X_Y->get_rank_mat( "desc" );
    VecF num = psprecision_k_helper( X_Y, score_rank_mat, K );
    VecF den = psprecision_k_helper( X_Y, true_rank_mat, K );

    VecF psprecision( K );
    for( int k=0; k<K; k++ )
        psprecision[k] = den[k]==0 ? 0 : num[k]/den[k];

    delete true_rank_mat;
    return psprecision;
}

VecF psnDCG_k_helper( SMatF* X_Y, SMatF* rank_mat, int K )
{
    int num_X = X_Y->nc;
    int num_Y = X_Y->nr;

    VecF labels( num_Y, 0 );
    VecF psnDCG( K, 0 );

    VecF coeffs( num_Y + 1 );
    coeffs[0] = 0;
    for( int i=1; i<=num_Y; i++ )
        coeffs[i] = (float) (1.0/log( i+1 ));

    for( int i=0; i<num_X; i++ )
    {
        for( int j=0; j<X_Y->size[i]; j++ )
        {
            int id = X_Y->data[i][j].first;
            float val = X_Y->data[i][j].second; 
            labels[ id ] = val;
        }

        for( int j=0; j<rank_mat->size[i]; j++ )
        {
            int id = rank_mat->data[i][j].first;
            int rank = (int)rank_mat->data[i][j].second;

            if( rank<=K && labels[id] )
            {
                for( int k=rank-1; k<K; k++ )
                    psnDCG[k] += labels[id] * coeffs[rank];
            }
        }

        for( int j=0; j<X_Y->size[i]; j++ )
            labels[ X_Y->data[i][j].first ] = 0;
    }

    return psnDCG;
}

VecF psnDCG_k( SMatF* X_Y, SMatF* score_rank_mat, int K )
{
    SMatF* true_rank_mat = X_Y->get_rank_mat( "desc" );
    VecF num = psnDCG_k_helper( X_Y, score_rank_mat, K );
    VecF den = psnDCG_k_helper( X_Y, true_rank_mat, K );

    VecF psnDCG( K );
    for( int k=0; k<K; k++ )
        psnDCG[k] = den[k]==0 ? 0 : num[k]/den[k];

    delete true_rank_mat;
    return psnDCG;
}

void print_all_metrics( SMatF* X_Y, SMatF* score_X_Y, int K )
{
    SMatF* score_rank_mat = score_X_Y->get_rank_mat( "desc" );

    VecF precision = precision_k( X_Y, score_rank_mat, K );
    cout << "P:";
    for( int k=0; k<K; k+=2 )
        cout << "\t" << k+1 << ":" << 100*precision[k] << "%";
    cout << endl;

    VecF psprecision = psprecision_k( X_Y, score_rank_mat, K );
    cout << "PSP:";
    for( int k=0; k<K; k+=2 )
        cout << "\t" << k+1 << ":" << 100*psprecision[k] << "%";
    cout << endl;

    VecF nDCG = nDCG_k( X_Y, score_rank_mat, K );
    cout << "N:";
    for( int k=0; k<K; k+=2 )
        cout << "\t" << k+1 << ":" << 100*nDCG[k] << "%";
    cout << endl;

    VecF psnDCG = psnDCG_k( X_Y, score_rank_mat, K );
    cout << "PSN:";
    for( int k=0; k<K; k+=2 )
        cout << "\t" << k+1 << ":" << 100*psnDCG[k] << "%";
    cout << endl;

    delete score_rank_mat;
}

void p_print_all_metrics( SMatF* X_Y, SMatF* score_X_Y, int K )
{
    print_all_metrics( X_Y, score_X_Y, K );
}