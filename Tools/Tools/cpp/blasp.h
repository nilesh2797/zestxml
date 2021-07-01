/* blasp.h  --  C prototypes for BLAS                         Ver 1.0 */
/* Jesse Bennett                                       March 23, 2000 */

/* Functions  listed in alphabetical order */

#pragma once

#ifdef F2C_COMPAT

void cdotc_(fcomplex *dotval, _int *n, fcomplex *cx, _int *incx,
            fcomplex *cy, _int *incy);

void cdotu_(fcomplex *dotval, _int *n, fcomplex *cx, _int *incx,
            fcomplex *cy, _int *incy);

_float sasum_(_int *n, _float *sx, _int *incx);

_float scasum_(_int *n, fcomplex *cx, _int *incx);

_float scnrm2_(_int *n, fcomplex *x, _int *incx);

_float sdot_(_int *n, _float *sx, _int *incx, _float *sy, _int *incy);

_float snrm2_(_int *n, _float *x, _int *incx);

void zdotc_(dcomplex *dotval, _int *n, dcomplex *cx, _int *incx,
            dcomplex *cy, _int *incy);

void zdotu_(dcomplex *dotval, _int *n, dcomplex *cx, _int *incx,
            dcomplex *cy, _int *incy);

#else

fcomplex cdotc_(_int *n, fcomplex *cx, _int *incx, fcomplex *cy, _int *incy);

fcomplex cdotu_(_int *n, fcomplex *cx, _int *incx, fcomplex *cy, _int *incy);

_float sasum_(_int *n, _float *sx, _int *incx);

_float scasum_(_int *n, fcomplex *cx, _int *incx);

_float scnrm2_(_int *n, fcomplex *x, _int *incx);

_float sdot_(_int *n, _float *sx, _int *incx, _float *sy, _int *incy);

_float snrm2_(_int *n, _float *x, _int *incx);

dcomplex zdotc_(_int *n, dcomplex *cx, _int *incx, dcomplex *cy, _int *incy);

dcomplex zdotu_(_int *n, dcomplex *cx, _int *incx, dcomplex *cy, _int *incy);

#endif

/* Remaining functions listed in alphabetical order */

_int caxpy_(_int *n, fcomplex *ca, fcomplex *cx, _int *incx, fcomplex *cy,
           _int *incy);

_int ccopy_(_int *n, fcomplex *cx, _int *incx, fcomplex *cy, _int *incy);

_int cgbmv_(char *trans, _int *m, _int *n, _int *kl, _int *ku,
           fcomplex *alpha, fcomplex *a, _int *lda, fcomplex *x, _int *incx,
           fcomplex *beta, fcomplex *y, _int *incy);

_int cgemm_(char *transa, char *transb, _int *m, _int *n, _int *k,
           fcomplex *alpha, fcomplex *a, _int *lda, fcomplex *b, _int *ldb,
           fcomplex *beta, fcomplex *c, _int *ldc);

_int cgemv_(char *trans, _int *m, _int *n, fcomplex *alpha, fcomplex *a,
           _int *lda, fcomplex *x, _int *incx, fcomplex *beta, fcomplex *y,
           _int *incy);

_int cgerc_(_int *m, _int *n, fcomplex *alpha, fcomplex *x, _int *incx,
           fcomplex *y, _int *incy, fcomplex *a, _int *lda);

_int cgeru_(_int *m, _int *n, fcomplex *alpha, fcomplex *x, _int *incx,
           fcomplex *y, _int *incy, fcomplex *a, _int *lda);

_int chbmv_(char *uplo, _int *n, _int *k, fcomplex *alpha, fcomplex *a,
           _int *lda, fcomplex *x, _int *incx, fcomplex *beta, fcomplex *y,
           _int *incy);

_int chemm_(char *side, char *uplo, _int *m, _int *n, fcomplex *alpha,
           fcomplex *a, _int *lda, fcomplex *b, _int *ldb, fcomplex *beta,
           fcomplex *c, _int *ldc);

_int chemv_(char *uplo, _int *n, fcomplex *alpha, fcomplex *a, _int *lda,
           fcomplex *x, _int *incx, fcomplex *beta, fcomplex *y, _int *incy);

_int cher_(char *uplo, _int *n, _float *alpha, fcomplex *x, _int *incx,
          fcomplex *a, _int *lda);

_int cher2_(char *uplo, _int *n, fcomplex *alpha, fcomplex *x, _int *incx,
           fcomplex *y, _int *incy, fcomplex *a, _int *lda);

_int cher2k_(char *uplo, char *trans, _int *n, _int *k, fcomplex *alpha,
            fcomplex *a, _int *lda, fcomplex *b, _int *ldb, _float *beta,
            fcomplex *c, _int *ldc);

_int cherk_(char *uplo, char *trans, _int *n, _int *k, _float *alpha,
           fcomplex *a, _int *lda, _float *beta, fcomplex *c, _int *ldc);

_int chpmv_(char *uplo, _int *n, fcomplex *alpha, fcomplex *ap, fcomplex *x,
           _int *incx, fcomplex *beta, fcomplex *y, _int *incy);

_int chpr_(char *uplo, _int *n, _float *alpha, fcomplex *x, _int *incx,
          fcomplex *ap);

_int chpr2_(char *uplo, _int *n, fcomplex *alpha, fcomplex *x, _int *incx,
           fcomplex *y, _int *incy, fcomplex *ap);

_int crotg_(fcomplex *ca, fcomplex *cb, _float *c, fcomplex *s);

_int cscal_(_int *n, fcomplex *ca, fcomplex *cx, _int *incx);

_int csscal_(_int *n, _float *sa, fcomplex *cx, _int *incx);

_int cswap_(_int *n, fcomplex *cx, _int *incx, fcomplex *cy, _int *incy);

_int csymm_(char *side, char *uplo, _int *m, _int *n, fcomplex *alpha,
           fcomplex *a, _int *lda, fcomplex *b, _int *ldb, fcomplex *beta,
           fcomplex *c, _int *ldc);

_int csyr2k_(char *uplo, char *trans, _int *n, _int *k, fcomplex *alpha,
            fcomplex *a, _int *lda, fcomplex *b, _int *ldb, fcomplex *beta,
            fcomplex *c, _int *ldc);

_int csyrk_(char *uplo, char *trans, _int *n, _int *k, fcomplex *alpha,
           fcomplex *a, _int *lda, fcomplex *beta, fcomplex *c, _int *ldc);

_int ctbmv_(char *uplo, char *trans, char *diag, _int *n, _int *k,
           fcomplex *a, _int *lda, fcomplex *x, _int *incx);

_int ctbsv_(char *uplo, char *trans, char *diag, _int *n, _int *k,
           fcomplex *a, _int *lda, fcomplex *x, _int *incx);

_int ctpmv_(char *uplo, char *trans, char *diag, _int *n, fcomplex *ap,
           fcomplex *x, _int *incx);

_int ctpsv_(char *uplo, char *trans, char *diag, _int *n, fcomplex *ap,
           fcomplex *x, _int *incx);

_int ctrmm_(char *side, char *uplo, char *transa, char *diag, _int *m,
           _int *n, fcomplex *alpha, fcomplex *a, _int *lda, fcomplex *b,
           _int *ldb);

_int ctrmv_(char *uplo, char *trans, char *diag, _int *n, fcomplex *a,
           _int *lda, fcomplex *x, _int *incx);

_int ctrsm_(char *side, char *uplo, char *transa, char *diag, _int *m,
           _int *n, fcomplex *alpha, fcomplex *a, _int *lda, fcomplex *b,
           _int *ldb);

_int ctrsv_(char *uplo, char *trans, char *diag, _int *n, fcomplex *a,
           _int *lda, fcomplex *x, _int *incx);

_int daxpy_(_int *n, _float *sa, _float *sx, _int *incx, _float *sy,
           _int *incy);

_int dcopy_(_int *n, _float *sx, _int *incx, _float *sy, _int *incy);

_int dgbmv_(char *trans, _int *m, _int *n, _int *kl, _int *ku,
           _float *alpha, _float *a, _int *lda, _float *x, _int *incx,
           _float *beta, _float *y, _int *incy);

_int dgemm_(char *transa, char *transb, _int *m, _int *n, _int *k,
           _float *alpha, _float *a, _int *lda, _float *b, _int *ldb,
           _float *beta, _float *c, _int *ldc);

_int dgemv_(char *trans, _int *m, _int *n, _float *alpha, _float *a,
           _int *lda, _float *x, _int *incx, _float *beta, _float *y,
           _int *incy);

_int dger_(_int *m, _int *n, _float *alpha, _float *x, _int *incx,
          _float *y, _int *incy, _float *a, _int *lda);

_int drot_(_int *n, _float *sx, _int *incx, _float *sy, _int *incy,
          _float *c, _float *s);

_int drotg_(_float *sa, _float *sb, _float *c, _float *s);

_int dsbmv_(char *uplo, _int *n, _int *k, _float *alpha, _float *a,
           _int *lda, _float *x, _int *incx, _float *beta, _float *y,
           _int *incy);

_int dscal_(_int *n, _float *sa, _float *sx, _int *incx);

_int dspmv_(char *uplo, _int *n, _float *alpha, _float *ap, _float *x,
           _int *incx, _float *beta, _float *y, _int *incy);

_int dspr_(char *uplo, _int *n, _float *alpha, _float *x, _int *incx,
          _float *ap);

_int dspr2_(char *uplo, _int *n, _float *alpha, _float *x, _int *incx,
           _float *y, _int *incy, _float *ap);

_int dswap_(_int *n, _float *sx, _int *incx, _float *sy, _int *incy);

_int dsymm_(char *side, char *uplo, _int *m, _int *n, _float *alpha,
           _float *a, _int *lda, _float *b, _int *ldb, _float *beta,
           _float *c, _int *ldc);

_int dsymv_(char *uplo, _int *n, _float *alpha, _float *a, _int *lda,
           _float *x, _int *incx, _float *beta, _float *y, _int *incy);

_int dsyr_(char *uplo, _int *n, _float *alpha, _float *x, _int *incx,
          _float *a, _int *lda);

_int dsyr2_(char *uplo, _int *n, _float *alpha, _float *x, _int *incx,
           _float *y, _int *incy, _float *a, _int *lda);

_int dsyr2k_(char *uplo, char *trans, _int *n, _int *k, _float *alpha,
            _float *a, _int *lda, _float *b, _int *ldb, _float *beta,
            _float *c, _int *ldc);

_int dsyrk_(char *uplo, char *trans, _int *n, _int *k, _float *alpha,
           _float *a, _int *lda, _float *beta, _float *c, _int *ldc);

_int dtbmv_(char *uplo, char *trans, char *diag, _int *n, _int *k,
           _float *a, _int *lda, _float *x, _int *incx);

_int dtbsv_(char *uplo, char *trans, char *diag, _int *n, _int *k,
           _float *a, _int *lda, _float *x, _int *incx);

_int dtpmv_(char *uplo, char *trans, char *diag, _int *n, _float *ap,
           _float *x, _int *incx);

_int dtpsv_(char *uplo, char *trans, char *diag, _int *n, _float *ap,
           _float *x, _int *incx);

_int dtrmm_(char *side, char *uplo, char *transa, char *diag, _int *m,
           _int *n, _float *alpha, _float *a, _int *lda, _float *b,
           _int *ldb);

_int dtrmv_(char *uplo, char *trans, char *diag, _int *n, _float *a,
           _int *lda, _float *x, _int *incx);

_int dtrsm_(char *side, char *uplo, char *transa, char *diag, _int *m,
           _int *n, _float *alpha, _float *a, _int *lda, _float *b,
           _int *ldb);

_int dtrsv_(char *uplo, char *trans, char *diag, _int *n, _float *a,
           _int *lda, _float *x, _int *incx);


_int saxpy_(_int *n, _float *sa, _float *sx, _int *incx, _float *sy, _int *incy);

_int scopy_(_int *n, _float *sx, _int *incx, _float *sy, _int *incy);

_int sgbmv_(char *trans, _int *m, _int *n, _int *kl, _int *ku,
           _float *alpha, _float *a, _int *lda, _float *x, _int *incx,
           _float *beta, _float *y, _int *incy);

_int sgemm_(char *transa, char *transb, _int *m, _int *n, _int *k,
           _float *alpha, _float *a, _int *lda, _float *b, _int *ldb,
           _float *beta, _float *c, _int *ldc);

_int sgemv_(char *trans, _int *m, _int *n, _float *alpha, _float *a,
           _int *lda, _float *x, _int *incx, _float *beta, _float *y,
           _int *incy);

_int sger_(_int *m, _int *n, _float *alpha, _float *x, _int *incx,
          _float *y, _int *incy, _float *a, _int *lda);

_int srot_(_int *n, _float *sx, _int *incx, _float *sy, _int *incy,
          _float *c, _float *s);

_int srotg_(_float *sa, _float *sb, _float *c, _float *s);

_int ssbmv_(char *uplo, _int *n, _int *k, _float *alpha, _float *a,
           _int *lda, _float *x, _int *incx, _float *beta, _float *y,
           _int *incy);

_int sscal_(_int *n, _float *sa, _float *sx, _int *incx);

_int sspmv_(char *uplo, _int *n, _float *alpha, _float *ap, _float *x,
           _int *incx, _float *beta, _float *y, _int *incy);

_int sspr_(char *uplo, _int *n, _float *alpha, _float *x, _int *incx,
          _float *ap);

_int sspr2_(char *uplo, _int *n, _float *alpha, _float *x, _int *incx,
           _float *y, _int *incy, _float *ap);

_int sswap_(_int *n, _float *sx, _int *incx, _float *sy, _int *incy);

_int ssymm_(char *side, char *uplo, _int *m, _int *n, _float *alpha,
           _float *a, _int *lda, _float *b, _int *ldb, _float *beta,
           _float *c, _int *ldc);

_int ssymv_(char *uplo, _int *n, _float *alpha, _float *a, _int *lda,
           _float *x, _int *incx, _float *beta, _float *y, _int *incy);

_int ssyr_(char *uplo, _int *n, _float *alpha, _float *x, _int *incx,
          _float *a, _int *lda);

_int ssyr2_(char *uplo, _int *n, _float *alpha, _float *x, _int *incx,
           _float *y, _int *incy, _float *a, _int *lda);

_int ssyr2k_(char *uplo, char *trans, _int *n, _int *k, _float *alpha,
            _float *a, _int *lda, _float *b, _int *ldb, _float *beta,
            _float *c, _int *ldc);

_int ssyrk_(char *uplo, char *trans, _int *n, _int *k, _float *alpha,
           _float *a, _int *lda, _float *beta, _float *c, _int *ldc);

_int stbmv_(char *uplo, char *trans, char *diag, _int *n, _int *k,
           _float *a, _int *lda, _float *x, _int *incx);

_int stbsv_(char *uplo, char *trans, char *diag, _int *n, _int *k,
           _float *a, _int *lda, _float *x, _int *incx);

_int stpmv_(char *uplo, char *trans, char *diag, _int *n, _float *ap,
           _float *x, _int *incx);

_int stpsv_(char *uplo, char *trans, char *diag, _int *n, _float *ap,
           _float *x, _int *incx);

_int strmm_(char *side, char *uplo, char *transa, char *diag, _int *m,
           _int *n, _float *alpha, _float *a, _int *lda, _float *b,
           _int *ldb);

_int strmv_(char *uplo, char *trans, char *diag, _int *n, _float *a,
           _int *lda, _float *x, _int *incx);

_int strsm_(char *side, char *uplo, char *transa, char *diag, _int *m,
           _int *n, _float *alpha, _float *a, _int *lda, _float *b,
           _int *ldb);

_int strsv_(char *uplo, char *trans, char *diag, _int *n, _float *a,
           _int *lda, _float *x, _int *incx);

_int zaxpy_(_int *n, dcomplex *ca, dcomplex *cx, _int *incx, dcomplex *cy,
           _int *incy);

_int zcopy_(_int *n, dcomplex *cx, _int *incx, dcomplex *cy, _int *incy);

_int zdscal_(_int *n, _float *sa, dcomplex *cx, _int *incx);

_int zgbmv_(char *trans, _int *m, _int *n, _int *kl, _int *ku,
           dcomplex *alpha, dcomplex *a, _int *lda, dcomplex *x, _int *incx,
           dcomplex *beta, dcomplex *y, _int *incy);

_int zgemm_(char *transa, char *transb, _int *m, _int *n, _int *k,
           dcomplex *alpha, dcomplex *a, _int *lda, dcomplex *b, _int *ldb,
           dcomplex *beta, dcomplex *c, _int *ldc);

_int zgemv_(char *trans, _int *m, _int *n, dcomplex *alpha, dcomplex *a,
           _int *lda, dcomplex *x, _int *incx, dcomplex *beta, dcomplex *y,
           _int *incy);

_int zgerc_(_int *m, _int *n, dcomplex *alpha, dcomplex *x, _int *incx,
           dcomplex *y, _int *incy, dcomplex *a, _int *lda);

_int zgeru_(_int *m, _int *n, dcomplex *alpha, dcomplex *x, _int *incx,
           dcomplex *y, _int *incy, dcomplex *a, _int *lda);

_int zhbmv_(char *uplo, _int *n, _int *k, dcomplex *alpha, dcomplex *a,
           _int *lda, dcomplex *x, _int *incx, dcomplex *beta, dcomplex *y,
           _int *incy);

_int zhemm_(char *side, char *uplo, _int *m, _int *n, dcomplex *alpha,
           dcomplex *a, _int *lda, dcomplex *b, _int *ldb, dcomplex *beta,
           dcomplex *c, _int *ldc);

_int zhemv_(char *uplo, _int *n, dcomplex *alpha, dcomplex *a, _int *lda,
           dcomplex *x, _int *incx, dcomplex *beta, dcomplex *y, _int *incy);

_int zher_(char *uplo, _int *n, _float *alpha, dcomplex *x, _int *incx,
          dcomplex *a, _int *lda);

_int zher2_(char *uplo, _int *n, dcomplex *alpha, dcomplex *x, _int *incx,
           dcomplex *y, _int *incy, dcomplex *a, _int *lda);

_int zher2k_(char *uplo, char *trans, _int *n, _int *k, dcomplex *alpha,
            dcomplex *a, _int *lda, dcomplex *b, _int *ldb, _float *beta,
            dcomplex *c, _int *ldc);

_int zherk_(char *uplo, char *trans, _int *n, _int *k, _float *alpha,
           dcomplex *a, _int *lda, _float *beta, dcomplex *c, _int *ldc);

_int zhpmv_(char *uplo, _int *n, dcomplex *alpha, dcomplex *ap, dcomplex *x,
           _int *incx, dcomplex *beta, dcomplex *y, _int *incy);

_int zhpr_(char *uplo, _int *n, _float *alpha, dcomplex *x, _int *incx,
          dcomplex *ap);

_int zhpr2_(char *uplo, _int *n, dcomplex *alpha, dcomplex *x, _int *incx,
           dcomplex *y, _int *incy, dcomplex *ap);

_int zrotg_(dcomplex *ca, dcomplex *cb, _float *c, dcomplex *s);

_int zscal_(_int *n, dcomplex *ca, dcomplex *cx, _int *incx);

_int zswap_(_int *n, dcomplex *cx, _int *incx, dcomplex *cy, _int *incy);

_int zsymm_(char *side, char *uplo, _int *m, _int *n, dcomplex *alpha,
           dcomplex *a, _int *lda, dcomplex *b, _int *ldb, dcomplex *beta,
           dcomplex *c, _int *ldc);

_int zsyr2k_(char *uplo, char *trans, _int *n, _int *k, dcomplex *alpha,
            dcomplex *a, _int *lda, dcomplex *b, _int *ldb, dcomplex *beta,
            dcomplex *c, _int *ldc);

_int zsyrk_(char *uplo, char *trans, _int *n, _int *k, dcomplex *alpha,
           dcomplex *a, _int *lda, dcomplex *beta, dcomplex *c, _int *ldc);

_int ztbmv_(char *uplo, char *trans, char *diag, _int *n, _int *k,
           dcomplex *a, _int *lda, dcomplex *x, _int *incx);

_int ztbsv_(char *uplo, char *trans, char *diag, _int *n, _int *k,
           dcomplex *a, _int *lda, dcomplex *x, _int *incx);

_int ztpmv_(char *uplo, char *trans, char *diag, _int *n, dcomplex *ap,
           dcomplex *x, _int *incx);

_int ztpsv_(char *uplo, char *trans, char *diag, _int *n, dcomplex *ap,
           dcomplex *x, _int *incx);

_int ztrmm_(char *side, char *uplo, char *transa, char *diag, _int *m,
           _int *n, dcomplex *alpha, dcomplex *a, _int *lda, dcomplex *b,
           _int *ldb);

_int ztrmv_(char *uplo, char *trans, char *diag, _int *n, dcomplex *a,
           _int *lda, dcomplex *x, _int *incx);

_int ztrsm_(char *side, char *uplo, char *transa, char *diag, _int *m,
           _int *n, dcomplex *alpha, dcomplex *a, _int *lda, dcomplex *b,
           _int *ldb);

_int ztrsv_(char *uplo, char *trans, char *diag, _int *n, dcomplex *a,
           _int *lda, dcomplex *x, _int *incx);
