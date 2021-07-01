#pragma once

#include <iostream>

#include "config.h"
#include "utils.h"
#include "mat.h"
#include "blas.h"

class tron_function
{
public:
	virtual _float fun( _float* w ) = 0 ;
	virtual void grad( _float* w, _float* g ) = 0 ;
	virtual void Hv( _float* s, _float* Hs ) = 0 ;

	virtual _int get_nr_variable( void ) = 0 ;
	virtual void get_diagH( _float* M ) = 0 ;
	virtual ~tron_function(void){}
};

class TRON
{
public:
	TRON( const tron_function* fun_obj, _float eps = 0.1, _float eps_cg = 0.1, _int max_iter = 1000 );
	~TRON();

	void tron( _float* w );

private:
	_int trpcg( _float delta, _float* g, _float* M, _float* s, _float* r, _bool* reach_boundary );
	_float norm_inf( _int n, _float* x );

	_float eps;
	_float eps_cg;
	_int max_iter;
	tron_function* fun_obj;
};
