#include "visual_inspection.h"

using namespace std;

void print_diff_visual_inspection( string vis_file, VecS& X, SMatF* score_mat1, SMatF* score_mat2, SMatF* X_Y, VecS& Y, bool weighted, int k, int diff_type )
{
	// diff_type=0 => only print examples where 1 is better than 2, diff_type=1 => only print examples where 2 is better than 1, diff_type=3 => union of cases 0 and 1

	assert( score_mat1->nc == score_mat2->nc );
	assert( score_mat1->nr == score_mat2->nr );

	int num_X = score_mat1->nc;
	int num_Y = score_mat1->nr;

	float* mask_Y = new float[ num_Y ]();

	ofstream fout;
	fout.open( vis_file );

	for( int i=0; i<num_X; i++ )
	{
		for( int j=0; j<X_Y->size[i]; j++ )
			mask_Y[ X_Y->data[i][j].first ] = weighted ? X_Y->data[i][j].second : 1.0;

		VecIF scores1;
		for( int j=0; j<score_mat1->size[i]; j++ )
			scores1.push_back( score_mat1->data[i][j] );
		sort( scores1.begin(), scores1.end(), comp_pair_by_second_desc<int,float> );

		VecIF scores2;
		for( int j=0; j<score_mat2->size[i]; j++ )
			scores2.push_back( score_mat2->data[i][j] );
		sort( scores2.begin(), scores2.end(), comp_pair_by_second_desc<int,float> );

		float sum_score1=0, sum_score2=0;

		for( int j=0; j<k; j++ )
		{
			if( j < scores1.size() )
				sum_score1 += mask_Y[ scores1[j].first ];
			if( j < scores2.size() )
				sum_score2 += mask_Y[ scores2[j].first ];
		}

		int pos_or_neg = 0;
		if( sum_score1 > sum_score2 + IRM_EPS )
			pos_or_neg = +1;
		else if( sum_score1 < sum_score2 - IRM_EPS )
			pos_or_neg = -1;

		if( (diff_type==0 && pos_or_neg==+1) || (diff_type==1 && pos_or_neg==-1) || (pos_or_neg!=0) )
		{
			fout << "diff_type: " << pos_or_neg << "\n";
			fout << "score1: " << sum_score1 << "\t" << "score2: " << sum_score2 << "\n";
			fout << i << "\t" << X[i] << "\n";
			fout << "true labels\n";
			for( int j=0; j<X_Y->size[i]; j++ )
				fout << "\t" << Y[ X_Y->data[i][j].first ] << ":" << mask_Y[ X_Y->data[i][j].first ] << "\n";

			fout << "scores1\n";
			for( int j=0; j<k; j++ )
				if( j < scores1.size() )
					fout << "\t" << Y[ scores1[j].first ] << ":" << scores1[j].second << ":" << mask_Y[ scores1[j].first ] << "\n";

			fout << "scores2\n";
			for( int j=0; j<k; j++ )
				if( j < scores2.size() )
					fout << "\t" << Y[ scores2[j].first ] << ":" << scores2[j].second << ":" << mask_Y[ scores2[j].first ] << "\n";

			fout << "\n";
		}

		for( int j=0; j<X_Y->size[i]; j++ )
			mask_Y[ X_Y->data[i][j].first ] = 0.0;
	}

	fout.close();

	delete [] mask_Y;
}

void visualize_true_predicted_labels( SMatF* X_Y, SMatF* score_mat, VecS& X, VecS& Y, string out_file, int max_X, int max_num_scores )
{
	cout << "Starting visualize_true_predicted_labels" << endl;
	ofstream fout;
	fout.open( out_file );

	int num_X = score_mat->nc;
	int num_Y = score_mat->nr;

	for( int i=0; i<min(num_X,max_X); i++ )
	{
		if( i% 1000 ==0 )
			cout << i << endl;

		fout << i << "\n" << X[i] << "\n";
		for( int j=0; j<X_Y->size[i]; j++ )
			fout << "\t" << j << "\t" << Y[ X_Y->data[i][j].first ] << "\t" << X_Y->data[i][j].second << "\n";
		int num_score = score_mat->size[i];
		sort( score_mat->data[i], score_mat->data[i] + num_score, comp_pair_by_second_desc<int,float> );
		for( int j=0; j<min(num_score,max_num_scores); j++ )
			fout << "\t\t" << j << "\t" << Y[ score_mat->data[i][j].first ] << "\t" << score_mat->data[i][j].second << "\n";
		fout << "\n";
		sort( score_mat->data[i], score_mat->data[i] + num_score, comp_pair_by_first<int,float> );
	}
	fout.close();
	cout << "Ending visualize_true_predicted_labels" << endl;
}

void visualize_predicted_labels( SMatF* score_mat, VecS& X, VecS& Y, string out_file, int max_X, int max_num_scores )
{
	ofstream fout;
	fout.open( out_file );

	int num_X = score_mat->nc;
	int num_Y = score_mat->nr;

	for( int i=0; i<min(num_X,max_X); i++ )
	{
		fout << i << "\n" << X[i] << "\n";

		int num_score = score_mat->size[i];
		sort( score_mat->data[i], score_mat->data[i] + num_score, comp_pair_by_second_desc<int,float> );
		for( int j=0; j<min(num_score,max_num_scores); j++ )
			fout << "\t" << Y[ score_mat->data[i][j].first ] << "\t\t" << score_mat->data[i][j].second << "\n";
		fout << "\n";
		sort( score_mat->data[i], score_mat->data[i] + num_score, comp_pair_by_first<int,float> );
	}

	fout.close();
}
