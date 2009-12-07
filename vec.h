#ifndef __VEC_H__
#define __VEC_H__

double mult_vec_vec(int n,const double* x,const double* y);

void mult_mat_vec(int n,int m,const double* x,const double* vec,double* res);

void vec_subtract(int n,const double* v1,const double* v2,double* to);

#endif
