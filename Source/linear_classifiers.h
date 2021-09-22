#pragma once

#include <iostream>
#include <string>
#include <cmath>

#include "config.h"
#include "utils.h"
#include "mat.h"
#include "timer.h"
#include "tron.h"

enum _Classifier_Kind { L2R_L2LOSS_SVC_DUAL=0, L2R_LR_DUAL, L2R_L2LOSS_SVC_PRIMAL, L2R_LR_PRIMAL };
const vector<string> _Classifier_Kind_Names = {"L2R_L2LOSS_SVC_DUAL", "L2R_LR_DUAL", "L2R_L2LOSS_SVC_PRIMAL", "L2R_LR_PRIMAL"};

class sparse_operator
{
public:
	static float nrm2_sq( int siz, pairIF* x )
	{
		float ret = 0;
		for( int i=0; i<siz; i++ )
		{
			ret += SQ( x[i].second );
		}
		return (ret);
	}

	static float dot( const float *s, int siz, pairIF* x )
	{
		float ret = 0;
		for( int i=0; i<siz; i++ )
			ret += s[ x[i].first ] * x[i].second;
		return (ret);
	}

	static void axpy(const float a, int siz, pairIF* x, float *y)
	{
		for( int i=0; i<siz; i++ )
		{
			y[x[i].first ] += a * x[i].second;
		}
	}
};

class l2r_lr_fun: public tron_function
{
public:
	l2r_lr_fun( SMatF* X_Xf, int* y, float* C );
	//l2r_lr_fun(const problem *prob, double *C);
	~l2r_lr_fun();

	float fun( float* w );
	void grad( float* w, float* g );
	void Hv( float* s, float* Hs );

	int get_nr_variable( void );
	void get_diagH( float* M );

private:
	void Xv( float* v, float* Xv );
	void XTv( float* v, float* XTv );

	float* C;
	float* z;
	float* D;
	SMatF* X_Xf;
	int* y;
};

/*
class l2r_l2_svc_fun: public tron_function
{
public:
	l2r_l2_svc_fun(const problem *prob, double *C);
	~l2r_l2_svc_fun();

	double fun(double *w);
	void grad(double *w, double *g);
	void Hv(double *s, double *Hs);

	int get_nr_variable(void);
	void get_diagH(double *M);

protected:
	void Xv(double *v, double *Xv);
	void subXTv(double *v, double *XTv);

	double *C;
	double *z;
	int *I;
	int sizeI;
	const problem *prob;
};
*/

void solve_l2r_lr_dual( SMatF* X_Xf, int* y, float *w, float eps, float* wts, int classifier_maxitr, mt19937& reng );
void solve_l2r_l2loss_svc_dual( SMatF* X_Xf, int* y, float *w, float eps, float* wts, int classifier_maxitr, mt19937& reng );
void solve_l2r_lr_primal( SMatF* X_Xf, int* y, float *w, float eps, float* wts, int classifier_maxitr, mt19937& reng );
//void solve_l2r_l2loss_svc_primal( SMatF* X_Xf, int* y, float *w, float eps, float* wts, int classifier_maxitr, mt19937& reng );
void solve_l1r_lr( SMatF* Xf_X, int* y, float *w, float eps, float* C, int classifier_maxitr, mt19937& reng );