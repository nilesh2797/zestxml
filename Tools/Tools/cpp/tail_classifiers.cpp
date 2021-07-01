#include "tail_classifiers.h"

using namespace std;

SMatF* tail_classifier_train( SMatF* trn_X_Xf, SMatF* trn_X_Y, float& train_time )
{
	train_time = 0;

	Timer timer;
	timer.tic();

	trn_X_Xf->unit_normalize_columns();

	SMatF* trn_Y_X = trn_X_Y->transpose();

	for(int i=0; i<trn_Y_X->nc; i++)
	{
		float a = trn_Y_X->size[i]==0 ? 1.0 : 1.0/(trn_Y_X->size[i]);
		for(int j=0; j<trn_Y_X->size[i]; j++)
			trn_Y_X->data[i][j].second = a;
	}

	SMatF* model_mat = trn_X_Xf->prod(trn_Y_X);
	model_mat->unit_normalize_columns();

	train_time += timer.toc();

	cout << "Tail classifiers training time: " << train_time/3600.0 << " hr" << endl;

	delete trn_Y_X;
	
	return model_mat;
}

void tail_classifier_predict( SMatF* tst_X_Xf, int x, VecIF &node_score_mat, SMatF* model_mat, float alpha, float& predict_time, float& model_size )
{
	// NOTE : assumption, tst_X_Xf is unit normalized + bias added

	int num_tst = tst_X_Xf->nc;
	int num_ft = tst_X_Xf->nr;
	int num_lbl = model_mat->nc;

	float* mask = new float[num_ft - 1]();

	// densify tst_X_Xf->data[x], NOTE : tst_X_Xf has extra bias feature that's why "i < tst_X_Xf->size[x]-1"
	for( int i = 0; i < tst_X_Xf->size[x]-1; i++ )
		mask[ (tst_X_Xf->data[x][i]).first ] = (tst_X_Xf->data[x][i]).second;

	for(int i = 0; i < node_score_mat.size(); i++)
	{
		int lbl = node_score_mat[i].first;
		float score = node_score_mat[i].second;

		float prod = mult_d_s_vec(mask, model_mat->data[lbl], model_mat->size[lbl]);

		node_score_mat[i].second = pow(score, alpha)*pow(exp(prod), 1-alpha);
	}

	// reset
	for( int i = 0; i < tst_X_Xf->size[x]-1; i++ )
		mask[ (tst_X_Xf->data[x][i]).first ] = 0;

	delete[] mask;
}

SMatF* tail_classifier_predict( SMatF* tst_X_Xf, SMatF* score_mat, SMatF* model_mat, float alpha, float threshold, float& predict_time, float& model_size )
{
	predict_time = 0;
	model_size = model_mat->get_ram();

	Timer timer;
	timer.tic();

	tst_X_Xf->unit_normalize_columns();

	int num_tst = tst_X_Xf->nc;
	int num_ft = tst_X_Xf->nr;
	int num_lbl = model_mat->nc;

	SMatF* tmat = score_mat->transpose();
	VecF mask(num_ft,0);

	for(int i=0; i<num_lbl; i++)
	{
		for(int j=0; j<model_mat->size[i]; j++)
			mask[model_mat->data[i][j].first] = model_mat->data[i][j].second;

		for(int j=0; j<tmat->size[i]; j++)
		{
			int inst = tmat->data[i][j].first;
			float prod = 0;
			for(int k=0; k<tst_X_Xf->size[inst]; k++)
			{
				int id = tst_X_Xf->data[inst][k].first;	
				float val = tst_X_Xf->data[inst][k].second;
				prod += mask[id]*val;
			}
			tmat->data[i][j].second = prod;
		}

		for(int j=0; j<model_mat->size[i]; j++)
			mask[model_mat->data[i][j].first] = 0;
	}

	SMatF* tail_score_mat = tmat->transpose();

	/* combine PfastXML scores and tail classifier scores to arrive at final scores */ 
	for(int i=0; i<num_tst; i++)
	{
		for(int j=0; j<score_mat->size[i]; j++)
		{
			float score = score_mat->data[i][j].second;
			float tail_score = tail_score_mat->data[i][j].second;
			tail_score_mat->data[i][j].second = fabs(tail_score)>threshold ? pow(score,alpha)*pow(tail_score,1-alpha) : 0.0;
		}
	}

	tail_score_mat->eliminate_zeros();

	predict_time += timer.toc();
	cout << "Tail classifiers prediction time: " << ((predict_time/tst_X_Xf->nc)*1000.0) << " ms/point" << endl;
	cout << "Tail classifiers model size: " << (model_size/1e+9) << " GB" << endl;

	delete tmat;

	return tail_score_mat;
}