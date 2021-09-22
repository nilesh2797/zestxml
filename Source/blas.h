/* blas.h  --  C header file for BLAS                         Ver 1.0 */
/* Jesse Bennett                                       March 23, 2000 */

/**  barf  [ba:rf]  2.  "He suggested using FORTRAN, and everybody barfed."

	- From The Shogakukan DICTIONARY OF NEW ENGLISH (Second edition) */

#pragma once

#include "config.h"
#include "utils.h"
#include "mat.h"

/* Data types specific to BLAS implementation */
typedef struct { float r, i; } fcomplex;
typedef struct { double r, i; } dcomplex;
typedef int blasbool;

//#include "blasp.h"    /* Prototypes for all BLAS functions */

#define FALSE 0
#define TRUE  1

/* Macro functions */
#define MIN(a,b) ((a) <= (b) ? (a) : (b))
#define MAX(a,b) ((a) >= (b) ? (a) : (b))

_float dnrm2_(_int *, _float *, _int *);
_float ddot_(_int *, _float *, _int *, _float *, _int *);
_int daxpy_(_int *, _float *, _float *, _int *, _float *, _int *);
_int dscal_(_int *, _float *, _float *, _int *);
