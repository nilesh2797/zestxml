#include <iostream>
#include <string>
#include <algorithm>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include "tron.h"

using namespace std;

static _float uTMv(_int n, _float *u, _float *M, _float *v)
{
	const _int m = n-4;
	_float res = 0;
	_int i;
	for (i=0; i<m; i+=5)
		res += u[i]*M[i]*v[i]+u[i+1]*M[i+1]*v[i+1]+u[i+2]*M[i+2]*v[i+2]+
			u[i+3]*M[i+3]*v[i+3]+u[i+4]*M[i+4]*v[i+4];
	for (; i<n; i++)
		res += u[i]*M[i]*v[i];
	return res;
}

TRON::TRON( const tron_function *fun_obj, _float eps, _float eps_cg, _int max_iter )
{
	this->fun_obj=const_cast<tron_function *>(fun_obj);
	this->eps=eps;
	this->eps_cg=eps_cg;
	this->max_iter=max_iter;
}

TRON::~TRON()
{
}

void TRON::tron(_float *w)
{
	// Parameters for updating the iterates.
	_float eta0 = 1e-4, eta1 = 0.25, eta2 = 0.75;
	// Parameters for updating the trust region size delta.
	_float sigma1 = 0.25, sigma2 = 0.5, sigma3 = 4;

	_int n = fun_obj->get_nr_variable();
	_int i, cg_iter;
	_float delta=0, sMnorm, one=1.0;
	_float alpha, f, fnew, prered, actred, gs;
	_int search = 1, iter = 1, inc = 1;
	_float *s = new _float[n];
	_float *r = new _float[n];
	_float *g = new _float[n];
	const _float alpha_pcg = 0.01;
	_float *M = new _float[n];

	// calculate gradient norm at w=0 for stopping condition.
	_float *w0 = new _float[n];
	for (i=0; i<n; i++)
		w0[i] = 0;
	fun_obj->fun(w0);
	fun_obj->grad(w0, g);
	_float gnorm0 = dnrm2_(&n, g, &inc);
	delete [] w0;
	f = fun_obj->fun(w);
	fun_obj->grad(w, g);
	_float gnorm = dnrm2_(&n, g, &inc);

	if (gnorm <= eps*gnorm0)
		search = 0;
	iter = 1;

	_float *w_new = new _float[n];
	_bool reach_boundary;
	while (iter <= max_iter && search)
	{
		fun_obj->get_diagH(M);

		for(i=0; i<n; i++)
			M[i] = (1-alpha_pcg) + alpha_pcg*M[i];
		if (iter == 1)
			delta = sqrt(uTMv(n, g, M, g));

		cg_iter = trpcg(delta, g, M, s, r, &reach_boundary);


		memcpy(w_new, w, sizeof(_float)*n);
		daxpy_(&n, &one, s, &inc, w_new, &inc);

		gs = ddot_(&n, g, &inc, s, &inc);
		prered = -0.5*(gs-ddot_(&n, s, &inc, r, &inc));
		fnew = fun_obj->fun(w_new);

		// Compute the actual reduction.
		actred = f - fnew;

		// On the first iteration, adjust the initial step bound.
		sMnorm = sqrt(uTMv(n, s, M, s));

		if (iter == 1)
			delta = min(delta, sMnorm);

		// Compute prediction alpha*sMnorm of the step.
		if (fnew - f - gs <= 0)
			alpha = sigma3;
		else
			alpha = max(sigma1, -(_float)(0.5*(gs/(fnew - f - gs))));

		// Update the trust region bound according to the ratio of actual to predicted reduction.
		if (actred < eta0*prered)
			delta = min(alpha*sMnorm, sigma2*delta);
		else if (actred < eta1*prered)
			delta = max(sigma1*delta, min(alpha*sMnorm, sigma2*delta));
		else if (actred < eta2*prered)
			delta = max(sigma1*delta, min(alpha*sMnorm, sigma3*delta));
		else
		{
			if (reach_boundary)
				delta = sigma3*delta;
			else
				delta = max(delta, min(alpha*sMnorm, sigma3*delta));
		}

		if (actred > eta0*prered)
		{
			iter++;
			memcpy(w, w_new, sizeof(_float)*n);
			f = fnew;
			fun_obj->grad(w, g);

			gnorm = dnrm2_(&n, g, &inc);
			if (gnorm <= eps*gnorm0)
				break;
		}
		if (f < -1.0e+32)
		{
			break;
		}
		if (prered <= 0)
		{
			break;
		}
		if (fabs(actred) <= 1.0e-12*fabs(f) &&
		    fabs(prered) <= 1.0e-12*fabs(f))
		{
			break;
		}
	}
	delete[] g;
	delete[] r;
	delete[] w_new;
	delete[] s;
	delete[] M;
}

_int TRON::trpcg(_float delta, _float *g, _float *M, _float *s, _float *r, _bool *reach_boundary)
{
	_int i, inc = 1;
	_int n = fun_obj->get_nr_variable();
	_float one = 1;
	_float *d = new _float[n];
	_float *Hd = new _float[n];
	_float zTr, znewTrnew, alpha, beta, cgtol;
	_float *z = new _float[n];
	*reach_boundary = false;
	for (i=0; i<n; i++)
	{
		s[i] = 0;
		r[i] = -g[i];
		z[i] = r[i] / M[i];
		d[i] = z[i];
	}
	zTr = ddot_(&n, z, &inc, r, &inc);

	cgtol = eps_cg*sqrt(zTr);
	_int cg_iter = 0;
	while (1)
	{
		if (sqrt(zTr) <= cgtol)
			break;
		cg_iter++;
		fun_obj->Hv(d, Hd);

		alpha = zTr/ddot_(&n, d, &inc, Hd, &inc);
		daxpy_(&n, &alpha, d, &inc, s, &inc);

		_float sMnorm = sqrt(uTMv(n, s, M, s));
		if (sMnorm > delta)
		{
			//cout << "cg reaches trust region boundary" << endl;
			*reach_boundary = true;
			alpha = -alpha;
			daxpy_(&n, &alpha, d, &inc, s, &inc);

			_float sTMd = uTMv(n, s, M, d);
			_float sTMs = uTMv(n, s, M, s);
			_float dTMd = uTMv(n, d, M, d);
			_float dsq = delta*delta;
			_float rad = sqrt(sTMd*sTMd + dTMd*(dsq-sTMs));
			if (sTMd >= 0)
				alpha = (dsq - sTMs)/(sTMd + rad);
			else
				alpha = (rad - sTMd)/dTMd;
			daxpy_(&n, &alpha, d, &inc, s, &inc);
			alpha = -alpha;
			daxpy_(&n, &alpha, Hd, &inc, r, &inc);
			break;
		}
		alpha = -alpha;
		daxpy_(&n, &alpha, Hd, &inc, r, &inc);

		for (i=0; i<n; i++)
			z[i] = r[i] / M[i];
		znewTrnew = ddot_(&n, z, &inc, r, &inc);
		beta = znewTrnew/zTr;
		dscal_(&n, &beta, d, &inc);
		daxpy_(&n, &one, z, &inc, d, &inc);
		zTr = znewTrnew;
	}
	delete[] d;
	delete[] Hd;
	delete[] z;
	return(cg_iter);
}

_float TRON::norm_inf(_int n, _float *x)
{
	_float dmax = fabs(x[0]);
	for (_int i=1; i<n; i++)
		if (fabs(x[i]) >= dmax)
			dmax = fabs(x[i]);
	return(dmax);
}
