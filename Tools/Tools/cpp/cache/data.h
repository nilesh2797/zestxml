#pragma once

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include "utils.h"
#include "mat.h"

using namespace std;

class Dataset{

public:
	SMatF* trn_X_Xf;
	SMatF* trn_X_Y;
	SMatF* tst_X_Xf;
	SMatF* tst_X_Y;
	SMatF* pattern_mat; // pattern matrix for prediction. Prediction scores will be evaluated only for non zero pairs of 'test point X label' in this matrix
	VecF inv_prop;

	Dataset()
	{
		trn_X_Xf = NULL;
		trn_X_Y = NULL;
		tst_X_Xf = NULL;
		tst_X_Y = NULL;
		pattern_mat = NULL;
	}

	// --dataset [dataset name] --split_no [split number] --trn_ft_file [training feature file name] --trn_lbl_file [training label file name] --tst_ft_file [test feature file name] --tst_lbl_file [test label file name]
	Dataset( int argc, char* argv[] )
	{
		trn_X_Xf = NULL;
		trn_X_Y = NULL;
		tst_X_Xf = NULL;
		tst_X_Y = NULL;
		pattern_mat = NULL;

		string dataset = "";
		int split_no = 0;
		string trn_ft_file = "";
		string trn_lbl_file = "";
		string tst_ft_file = "";
		string tst_lbl_file = "";
		string pattern_file = "";
		int num_Xf = -1;
		int num_Y = -1;
		string opt;
		string sval;
		float val;

		for( int i=0; i<argc; i+=2 )
		{
			opt = string(argv[i]);
			sval = string(argv[i+1]);

			if( opt=="--dataset" )
				dataset = sval;
			else if( opt=="--split_no" )
				split_no = stoi( sval );
			else if( opt=="--trn_ft_file" )
				trn_ft_file = sval;
			else if( opt=="--trn_lbl_file" )
				trn_lbl_file = sval;
			else if( opt=="--tst_ft_file" )
				tst_ft_file = sval;
			else if( opt=="--tst_lbl_file" )
				tst_lbl_file = sval;
			else if( opt=="--pattern_file" )
				pattern_file = sval;
			else if( opt=="--num_Xf" )
				num_Xf = stoi( sval );
			else if( opt=="--num_Y" )
				num_Y = stoi( sval );
		}

		if( dataset != "" )
		{
			string EXP_DIR = string( getenv( "EXP_DIR" ) );
			string data_dir;

			if( dataset=="MillionSong-1M" || dataset=="YahooMovie-8K" )
				data_dir = EXP_DIR + "/Datasets/Recommendation/" + dataset + "/";
			else if( dataset=="DSA-200K" || dataset=="DSA-7M" )
				data_dir = EXP_DIR + "/Datasets/DSA/" + dataset + "/";
			else
				data_dir = EXP_DIR + "/Datasets/MultiLabel/" + dataset + "/";

			SMatF* X_Xf = new SMatF( data_dir + "X_Xf.txt" );
			SMatF* X_Y = new SMatF( data_dir + "X_Y.txt" );

			bool* split = read_vec<bool>( data_dir + "split." + to_string(split_no) + ".txt" );
			X_Xf->split_mat( split, trn_X_Xf, tst_X_Xf );
			X_Y->split_mat( split, trn_X_Y, tst_X_Y );

			delete X_Xf;
			delete X_Y;
			delete [] split;
		}
		else
		{
			trn_X_Xf = NULL;
			trn_X_Y = NULL;
			tst_X_Xf = NULL;
			tst_X_Y = NULL;

			if( trn_ft_file != "" )
				trn_X_Xf = new SMatF( trn_ft_file );

			if( trn_lbl_file != "" )
				trn_X_Y = new SMatF( trn_lbl_file );

			if( tst_ft_file != "" )
				tst_X_Xf = new SMatF( tst_ft_file );

			if( tst_lbl_file != "" )
				tst_X_Y = new SMatF( tst_lbl_file );

		}

		if( pattern_file != "" )
			pattern_mat = new SMatF( pattern_file );

		// Inverse propensity score calculation

		if( trn_X_Y != NULL )
		{
			float A = 0.55;
			float B = 1.5;

			if( dataset == "WikiLSHTC-325K" )
			{
				A = 0.5;
				B = 0.4;
			}
			else if( dataset == "Amazon-670K" )
			{
				A = 0.6;
				B = 2.6;
			}

			int num_X = trn_X_Y->nc;
			int num_Y = trn_X_Y->nr;
			float C = (log((_double)num_X)-1)*pow(B+1,A);

			int* freqs = new int[ num_Y ]();
			for( int i=0; i<num_X; i++ )
				for( int j=0; j<trn_X_Y->size[i]; j++ )
					freqs[ trn_X_Y->data[i][j].first ]++;

			for( int i=0; i<num_Y; i++ )
				inv_prop.push_back( 1 + C* ( pow( freqs[i]+B, -A ) ) );

			delete [] freqs;
		}
	}

	Dataset( string dataset, int split_no=0 )
	{
		trn_X_Xf = NULL;
		trn_X_Y = NULL;
		tst_X_Xf = NULL;
		tst_X_Y = NULL;
		pattern_mat = NULL;

		string EXP_DIR = string( getenv( "EXP_DIR" ) );
		string data_dir = EXP_DIR + "/Datasets/MultiLabel/" + dataset + "/";
		SMatF* X_Xf = new SMatF( data_dir + "X_Xf.txt" );
		SMatF* X_Y = new SMatF( data_dir + "X_Y.txt" );

		bool* split = read_vec<bool>( data_dir + "split." + to_string(split_no) + ".txt" );
		X_Xf->split_mat( split, trn_X_Xf, tst_X_Xf );
		X_Y->split_mat( split, trn_X_Y, tst_X_Y );

		delete X_Xf;
		delete X_Y;
		delete [] split;

		// Inverse propensity score calculation

		float A = 0.55;
		float B = 1.5;

		if( dataset == "WikiLSHTC-325K" )
		{
			A = 0.5;
			B = 0.4;
		}
		else if( dataset == "Amazon-670K" )
		{
			A = 0.6;
			B = 2.6;
		}

		int num_X = trn_X_Y->nc;
		int num_Y = trn_X_Y->nr;
		float C = (log((_double)num_X)-1)*pow(B+1,A);

		int* freqs = new int[ num_Y ]();
		for( int i=0; i<num_X; i++ )
			for( int j=0; j<trn_X_Y->size[i]; j++ )
				freqs[ trn_X_Y->data[i][j].first ]++;
		
		for( int i=0; i<num_Y; i++ )
			inv_prop.push_back( 1 + C* ( pow( freqs[i]+B, -A ) ) );

		delete [] freqs;
	}

	~Dataset()
	{
		delete trn_X_Xf;
		delete trn_X_Y;
		delete tst_X_Xf;
		delete tst_X_Y;
		delete pattern_mat;
	}
};

VecS read_desc_file( string fname );
void print_descs( VecS descs );