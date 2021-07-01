#include "linear_classifiers.h"

#define GETI(i) (y[i]+1)

void solve_l1r_lr( SMatF* Xf_X, int* y, float *w, float eps, float* C, int classifier_maxitr, mt19937& reng )
{
    int num_X = Xf_X->nr;
    int num_Xf = Xf_X->nc;
    vector<int>& size = Xf_X->size;
    vector<pairIF*>& data = Xf_X->data;
    
	int l = num_X;
	int w_size = num_Xf;
	int newton_iter=0, iter=0;
	int max_newton_iter = classifier_maxitr;
	int max_iter = 10;
	int max_num_linesearch = 20;
	int active_size;
	int QP_active_size;

	double nu = 1e-12;
	double inner_eps = 1;
	double sigma = 0.01;
	double w_norm, w_norm_new;
	double z, G, H;
	double Gnorm1_init;
	double Gmax_old = INF;
	double Gmax_new, Gnorm1_new;
	double QP_Gmax_old = INF;
	double QP_Gmax_new, QP_Gnorm1_new;
	double delta, negsum_xTd, cond;

    VecI index( num_Xf, 0 );
    VecD Hdiag( num_Xf, 0 );
    VecD Grad( num_Xf, 0 );
    VecD wpd( num_Xf, 0 );
    VecD xjneg_sum( num_Xf, 0 );
    VecD xTd( num_X, 0 );
    VecD exp_wTx( num_X, 0 );
    VecD exp_wTx_new( num_X, 0 );
    VecD tau( num_X, 0 );
    VecD D( num_X, 0 );
	
	w_norm = 0;
	for( int i=0; i<w_size; i++ )
	{
		index[i] = i;

        for( int j=0; j<size[i]; j++ )
		{
			int inst = data[i][j].first;
			float val = data[i][j].second;

			if(y[inst] == -1)
				xjneg_sum[i] += C[inst]*val;
		}
	}

	for( int i=0; i<l; i++ )
	{
		exp_wTx[i] = exp(exp_wTx[i]);
		double tau_tmp = 1/(1+exp_wTx[i]);
		tau[i] = C[i]*tau_tmp;
		D[i] = C[i]*exp_wTx[i]*SQ(tau_tmp);
	}

	while(newton_iter < max_newton_iter)
	{
		Gmax_new = 0;
		Gnorm1_new = 0;
		active_size = w_size;

		for(int s=0; s<active_size; s++)
		{
			int i = index[s];
			Hdiag[i] = nu;

			double tmp = 0;
		
			for( int j=0; j<size[i]; j++ )
			{
				int inst = data[i][j].first;
				float val = data[i][j].second;
				Hdiag[i] += SQ(val)*D[inst];
				tmp += val*tau[inst];
			}

			Grad[i] = -tmp + xjneg_sum[i];

			double Gp = Grad[i]+1;
			double Gn = Grad[i]-1;
			double violation = 0;

			if(w[i] == 0)
			{
				if(Gp < 0)
					violation = -Gp;
				else if(Gn > 0)
					violation = Gn;
				//outer-level shrinking
				else if(Gp>Gmax_old/l && Gn<-Gmax_old/l)
				{
					active_size--;
					swap(index[s], index[active_size]);
					s--;
					continue;
				}
			}
			else if(w[i] > 0)
				violation = fabs(Gp);
			else
				violation = fabs(Gn);

			Gmax_new = max(Gmax_new, violation);
			Gnorm1_new += violation;
		}

		if(newton_iter == 0)
			Gnorm1_init = Gnorm1_new;

		if(Gnorm1_new <= eps*Gnorm1_init)
			break;

		iter = 0;
		QP_Gmax_old = INF;
		QP_active_size = active_size;

		for(int i=0; i<l; i++)
			xTd[i] = 0;

		// optimize QP over wpd
		while(iter < max_iter)
		{
			QP_Gmax_new = 0;
			QP_Gnorm1_new = 0;

			for(int i=0; i<QP_active_size; i++)
			{
				//_llint r = reng();
				//int j = i+r%(QP_active_size-i);
				int j = i + get_rand_num( QP_active_size-i, reng );
				swap(index[j], index[i]);
			}

			for(int s=0; s<QP_active_size; s++)
			{
				int i = index[s];
				H = Hdiag[i];

				G = Grad[i] + (wpd[i]-w[i])*nu;
				for( int j=0; j<size[i]; j++ )
				{
					int inst = data[i][j].first;
					float val = data[i][j].second;
					G += val*D[inst]*xTd[inst];
				}

				double Gp = G+1;
				double Gn = G-1;
				double violation = 0;
				if(wpd[i] == 0)
				{
					if(Gp < 0)
						violation = -Gp;
					else if(Gn > 0)
						violation = Gn;
					//inner-level shrinking
					else if(Gp>QP_Gmax_old/l && Gn<-QP_Gmax_old/l)
					{
						QP_active_size--;
						swap(index[s], index[QP_active_size]);
						s--;
						continue;
					}
				}
				else if(wpd[i] > 0)
					violation = fabs(Gp);
				else
					violation = fabs(Gn);

				QP_Gmax_new = max(QP_Gmax_new, violation);
				QP_Gnorm1_new += violation;

				// obtain solution of one-variable problem
				if(Gp < H*wpd[i])
					z = -Gp/H;
				else if(Gn > H*wpd[i])
					z = -Gn/H;
				else
					z = -wpd[i];

				if(fabs(z) < 1.0e-12)
					continue;
				z = min(max(z,-10.0),10.0);

				wpd[i] += z;

				for( int j=0; j<size[i]; j++ )
				{
					int inst = data[i][j].first;
					float val = data[i][j].second;
					xTd[inst] += val*z;
				}
			}

			iter++;

			if(QP_Gnorm1_new <= inner_eps*Gnorm1_init)
			{
				//inner stopping
				if(QP_active_size == active_size)
					break;
				//active set reactivation
				else
				{
					QP_active_size = active_size;
					QP_Gmax_old = INF;
					continue;
				}
			}

			QP_Gmax_old = QP_Gmax_new;
		}

		delta = 0;
		w_norm_new = 0;
		for(int i=0; i<w_size; i++)
		{
			delta += Grad[i]*(wpd[i]-w[i]);
			if(wpd[i] != 0)
				w_norm_new += fabs(wpd[i]);
		}
		delta += (w_norm_new-w_norm);

		negsum_xTd = 0;
		for(int i=0; i<l; i++)
		{
			if(y[i] == -1)
				negsum_xTd += C[i]*xTd[i];
		}

		int num_linesearch;
		for(num_linesearch=0; num_linesearch < max_num_linesearch; num_linesearch++)
		{
			double cond = w_norm_new - w_norm + negsum_xTd - sigma*delta;

			for(int i=0; i<l; i++)
			{
				double exp_xTd = exp(xTd[i]);
				exp_wTx_new[i] = exp_wTx[i]*exp_xTd;
				cond += C[i]*log((1+exp_wTx_new[i])/(exp_xTd+exp_wTx_new[i]));
			}

			if(cond <= 0)
			{
				w_norm = w_norm_new;
				for(int i=0; i<w_size; i++)
					w[i] = wpd[i];

				for(int i=0; i<l; i++)
				{
					exp_wTx[i] = exp_wTx_new[i];
					double tau_tmp = 1/(1+exp_wTx[i]);
					tau[i] = C[i]*tau_tmp;
					D[i] = C[i]*exp_wTx[i]*SQ(tau_tmp);
				}
				break;
			}
			else
			{
				w_norm_new = 0;
				for(int i=0; i<w_size; i++)
				{
					wpd[i] = (w[i]+wpd[i])*0.5;

					if(wpd[i] != 0)
						w_norm_new += fabs(wpd[i]);
				}
				delta *= 0.5;
				negsum_xTd *= 0.5;
				for(int i=0; i<l; i++)
					xTd[i] *= 0.5;
			}
		}

		// Recompute some info due to too many line search steps
		if(num_linesearch >= max_num_linesearch)
		{
			for(int i=0; i<l; i++)
				exp_wTx[i] = 0;

			for(int i=0; i<w_size; i++)
			{
				if(w[i]==0) continue;

				for( int j=0; j<size[i]; j++ )
				{
					int inst = data[i][j].first;
					float val = data[i][j].second;
					exp_wTx[inst] += w[i]*val;
				}
			}

			for(int i=0; i<l; i++)
				exp_wTx[i] = exp(exp_wTx[i]);
		}

		if(iter == 1)
			inner_eps *= 0.25;

		newton_iter++;
		Gmax_old = Gmax_new;
	}
}

void solve_l2r_lr_dual( SMatF* X_Xf, int* y, float *w, float eps, float* wts, int classifier_maxitr, mt19937& reng )
{
	int l = X_Xf->nc;
	int w_size = X_Xf->nr;
	int i, s, iter = 0;

	double *xTx = new double[l];
	int max_iter = classifier_maxitr;
	int *index = new int[l];	
	double *alpha = new double[2*l]; // store alpha and C - alpha
	int max_inner_iter = 100; // for inner Newton
	double innereps = 1e-2;
	double innereps_min = min(1e-8, (double)eps);

	vector<int>& size = X_Xf->size;
	vector<pairIF*>& data = X_Xf->data;

	// Initial alpha can be set here. Note that
	for(i=0; i<l; i++)
	{
		alpha[2*i] = min(0.001*wts[i], 1e-8);
		alpha[2*i+1] = wts[i] - alpha[2*i];
	}

	for(i=0; i<l; i++)
	{
		xTx[i] = sparse_operator::nrm2_sq( size[i], data[i] );
		sparse_operator::axpy(y[i]*alpha[2*i], size[i], data[i], w);
		index[i] = i;
	}

	while (iter < max_iter)
	{
		for (i=0; i<l; i++)
		{
			int j = i + get_rand_num( l-i, reng );
			swap(index[i], index[j]);
		}

		int newton_iter = 0;
		double Gmax = 0;
		for (s=0; s<l; s++)
		{
			i = index[s];
			const int yi = y[i];
			double C = wts[i];
			double ywTx = 0, xisq = xTx[i];
			ywTx = yi*sparse_operator::dot( w, size[i], data[i] );
			double a = xisq, b = ywTx;

			// Decide to minimize g_1(z) or g_2(z)
			int ind1 = 2*i, ind2 = 2*i+1, sign = 1;
			if(0.5*a*(alpha[ind2]-alpha[ind1])+b < 0)
			{
				ind1 = 2*i+1;
				ind2 = 2*i;
				sign = -1;
			}

			double alpha_old = alpha[ind1];
			double z = alpha_old;
			if(C - z < 0.5 * C)
				z = 0.1*z;
			double gp = a*(z-alpha_old)+sign*b+log(z/(C-z));
			Gmax = max(Gmax, fabs(gp));

			// Newton method on the sub-problem
			const double eta = 0.1; // xi in the paper
			int inner_iter = 0;
			while (inner_iter <= max_inner_iter)
			{
				if(fabs(gp) < innereps)
					break;
				double gpp = a + C/(C-z)/z;
				double tmpz = z - gp/gpp;
				if(tmpz <= 0)
					z *= eta;
				else // tmpz in (0, C)
					z = tmpz;
				gp = a*(z-alpha_old)+sign*b+log(z/(C-z));
				newton_iter++;
				inner_iter++;
			}

			if(inner_iter > 0) // update w
			{
				alpha[ind1] = z;
				alpha[ind2] = C-z;
				sparse_operator::axpy(sign*(z-alpha_old)*yi, size[i], data[i], w);
			}
		}

		iter++;

		if(Gmax < eps)
			break;

		if(newton_iter <= l/10)
			innereps = max(innereps_min, 0.1*innereps);

	}

	delete [] xTx;
	delete [] alpha;
	delete [] index;
}

void solve_l2r_l2loss_svc_dual( SMatF* X_Xf, int* y, float *w, float eps, float* wts, int classifier_maxitr, mt19937& reng )
{
	int l = X_Xf->nc;
	int w_size = X_Xf->nr;

	int i, s, iter = 0;
	float C, d, G;
	float *QD = new float[l];
	int max_iter = classifier_maxitr;
	int *index = new int[l];
	float *alpha = new float[l];
	int active_size = l;

	int tot_iter = 0;

	// PG: projected gradient, for shrinking and stopping
	float PG;
	float PGmax_old = INF;
	float PGmin_old = -INF;
	float PGmax_new, PGmin_new;

	// default solver_type: L2R_L2LOSS_SVC_DUAL

	float* diag = new float[l];
        for( int i=0; i<l; i++ )
                diag[i] = (float)0.5/wts[i];
	float upper_bound[3] = {INF, 0, INF};

	vector<int>& size = X_Xf->size;
	vector<pairIF*>& data = X_Xf->data;

	//d = pwd;
	//Initial alpha can be set here. Note that
	// 0 <= alpha[i] <= upper_bound[GETI(i)]

	for(i=0; i<l; i++)
		alpha[i] = 0;

	for(i=0; i<l; i++)
	{
		QD[i] = diag[i];
		QD[i] += sparse_operator::nrm2_sq( size[i], data[i] );
		sparse_operator::axpy(y[i]*alpha[i], size[i], data[i], w);
		index[i] = i;
	}

	while (iter < max_iter)
	{
		PGmax_new = -INF;
		PGmin_new = INF;

		for (i=0; i<active_size; i++)
		{
			int j = i + get_rand_num( active_size-i, reng );
			swap(index[i], index[j]);
		}

		for (s=0; s<active_size; s++)
		{
			tot_iter ++;

			i = index[s];
			const int yi = y[i];

			G = yi*sparse_operator::dot( w, size[i], data[i] )-1;

			C = upper_bound[GETI(i)];
			G += alpha[i]*diag[i];

			PG = 0;
			if (alpha[i] == 0)
			{
				if (G > PGmax_old)
				{
					active_size--;
					swap(index[s], index[active_size]);
					s--;
					continue;
				}
				else if (G < 0)
					PG = G;
			}
			else if (alpha[i] == C)
			{
				if (G < PGmin_old)
				{
					active_size--;
					swap(index[s], index[active_size]);
					s--;
					continue;
				}
				else if (G > 0)
					PG = G;
			}
			else
				PG = G;

			PGmax_new = max(PGmax_new, PG);
			PGmin_new = min(PGmin_new, PG);

			if(fabs(PG) > 1.0e-12)
			{
				float alpha_old = alpha[i];
				alpha[i] = min(max(alpha[i] - G/QD[i], (float)0.0), C);
				d = (alpha[i] - alpha_old)*yi;
				sparse_operator::axpy(d, size[i], data[i], w);
			}
		}

		iter++;

		if(PGmax_new - PGmin_new <= eps)
		{
			if(active_size == l)
				break;
			else
			{
				active_size = l;
				PGmax_old = INF;
				PGmin_old = -INF;
				continue;
			}
		}
		PGmax_old = PGmax_new;
		PGmin_old = PGmin_new;
		if (PGmax_old <= 0)
			PGmax_old = INF;
		if (PGmin_old >= 0)
			PGmin_old = -INF;
	}

	// calculate objective value

	delete [] diag;
	delete [] QD;
	delete [] alpha;
	delete [] index;
}

/*
l2r_lr_fun::l2r_lr_fun( SMatF* X_Xf, int* y, float* C )
{
	this->X_Xf = X_Xf;
	this->y = y;
	int l = X_Xf->nc;
	z = new float[ l ];
	D = new float[ l ];
	this->C = C;
}

l2r_lr_fun::~l2r_lr_fun()
{
	delete[] z;
	delete[] D;
}

float l2r_lr_fun::fun( float* w )
{
	float f=0;
	int l = X_Xf->nc;
	int w_size = X_Xf->nr;

	Xv(w, z);

	for( int i=0; i<w_size; i++ )
		f += w[i]*w[i];
	f /= 2.0;

	for( int i=0; i<l; i++ )
	{
		float yz = y[i]*z[i];
		if (yz >= 0)
			f += C[i]*log(1 + exp(-yz));
		else
			f += C[i]*(-yz+log(1 + exp(yz)));
	}

	return f;
}

void l2r_lr_fun::grad( float* w, float* g )
{
	int l = X_Xf->nc;
	int w_size = X_Xf->nr;

	for( int i=0; i<l; i++ )
	{
		z[i] = 1/(1 + exp(-y[i]*z[i]));
		D[i] = z[i]*(1-z[i]);
		z[i] = C[i]*(z[i]-1)*y[i];
	}
	XTv(z, g);

	for( int i=0; i<w_size; i++ )
		g[i] = w[i] + g[i];
}

int l2r_lr_fun::get_nr_variable(void)
{
	return X_Xf->nr;
}

void l2r_lr_fun::Hv( float* s, float* Hs )
{
	int l = X_Xf->nc;
	int w_size = X_Xf->nr;

	for( int i=0; i<w_size; i++ )
		Hs[i] = 0;

	for( int i=0; i<l; i++ )
	{
		float xTs = sparse_operator::dot( s, X_Xf->size[i], X_Xf->data[i] );
		xTs = C[i]*D[i]*xTs;
		sparse_operator::axpy( xTs, X_Xf->size[i], X_Xf->data[i], Hs );
	}
	for( int i=0; i<w_size; i++ )
		Hs[i] = s[i] + Hs[i];
}

void l2r_lr_fun::get_diagH( float* M )
{
	int l = X_Xf->nc;
	int w_size = X_Xf->nr;

	for ( int i=0; i<w_size; i++ )
		M[i] = 1;

	for ( int i=0; i<l; i++)
	{
		for( int j=0; j<X_Xf->size[i]; j++ )
		{
			int id = X_Xf->data[i][j].first;
			float val = X_Xf->data[i][j].second;
			M[ id ] += SQ( val )*C[i]*D[i];
		}
	}
}

void l2r_lr_fun::Xv( float* v, float* Xv )
{
	int l = X_Xf->nc;
	int w_size = X_Xf->nr;

	for( int i=0; i<l; i++ )
		Xv[i] = sparse_operator::dot( v, X_Xf->size[i], X_Xf->data[i] );
}

void l2r_lr_fun::XTv( float* v, float* XTv )
{
	int l = X_Xf->nc;
	int w_size = X_Xf->nr;

	for( int i=0; i<w_size; i++ )
		XTv[i]=0;

	for( int i=0; i<l; i++ )
		sparse_operator::axpy( v[i], X_Xf->size[i], X_Xf->data[i], XTv );
}

void solve_l2r_lr_primal( SMatF* X_Xf, int* y, float *w, float eps, float* wts, int classifier_maxitr, mt19937& reng ) // Default value of eps is 0.1
{
	l2r_lr_fun* fun_obj = new l2r_lr_fun( X_Xf, y, wts );
	TRON tron_obj( fun_obj, 0.05, 0.05 );
	tron_obj.tron( w );
	delete fun_obj;
}
*/

/*
l2r_l2_svc_fun::l2r_l2_svc_fun(const problem *prob, double *C)
{
	int l=prob->l;

	this->prob = prob;

	z = new double[l];
	I = new int[l];
	this->C = C;
}

l2r_l2_svc_fun::~l2r_l2_svc_fun()
{
	delete[] z;
	delete[] I;
}

double l2r_l2_svc_fun::fun(double *w)
{
	int i;
	double f=0;
	double *y=prob->y;
	int l=prob->l;
	int w_size=get_nr_variable();

	Xv(w, z);

	for(i=0;i<w_size;i++)
		f += w[i]*w[i];
	f /= 2.0;
	for(i=0;i<l;i++)
	{
		z[i] = y[i]*z[i];
		double d = 1-z[i];
		if (d > 0)
			f += C[i]*d*d;
	}

	return(f);
}

void l2r_l2_svc_fun::grad(double *w, double *g)
{
	int i;
	double *y=prob->y;
	int l=prob->l;
	int w_size=get_nr_variable();

	sizeI = 0;
	for (i=0;i<l;i++)
		if (z[i] < 1)
		{
			z[sizeI] = C[i]*y[i]*(z[i]-1);
			I[sizeI] = i;
			sizeI++;
		}
	subXTv(z, g);

	for(i=0;i<w_size;i++)
		g[i] = w[i] + 2*g[i];
}

int l2r_l2_svc_fun::get_nr_variable(void)
{
	return prob->n;
}

void l2r_l2_svc_fun::get_diagH(double *M)
{
	int i;
	int w_size=get_nr_variable();
	feature_node **x = prob->x;

	for (i=0; i<w_size; i++)
		M[i] = 1;

	for (i=0; i<sizeI; i++)
	{
		int idx = I[i];
		feature_node *s = x[idx];
		while (s->index!=-1)
		{
			M[s->index-1] += s->value*s->value*C[idx]*2;
			s++;
		}
	}
}

void l2r_l2_svc_fun::Hv(double *s, double *Hs)
{
	int i;
	int w_size=get_nr_variable();
	feature_node **x=prob->x;

	for(i=0;i<w_size;i++)
		Hs[i]=0;
	for(i=0;i<sizeI;i++)
	{
		feature_node * const xi=x[I[i]];
		double xTs = sparse_operator::dot(s, xi);

		xTs = C[I[i]]*xTs;

		sparse_operator::axpy(xTs, xi, Hs);
	}
	for(i=0;i<w_size;i++)
		Hs[i] = s[i] + 2*Hs[i];
}

void l2r_l2_svc_fun::Xv(double *v, double *Xv)
{
	int i;
	int l=prob->l;
	feature_node **x=prob->x;

	for(i=0;i<l;i++)
		Xv[i]=sparse_operator::dot(v, x[i]);
}

void l2r_l2_svc_fun::subXTv(double *v, double *XTv)
{
	int i;
	int w_size=get_nr_variable();
	feature_node **x=prob->x;

	for(i=0;i<w_size;i++)
		XTv[i]=0;
	for(i=0;i<sizeI;i++)
		sparse_operator::axpy(v[i], x[I[i]], XTv);
}
*/