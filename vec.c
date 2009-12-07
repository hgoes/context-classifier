#include "vec.h"

double mult_vec_vec(int n,const double* x,const double* y) {
  int i;
  double res = 0;
  for(i=0;i<n;i++) {
    res+=x[i]*y[i];
  }
  return res;
}

void mult_mat_vec(int n,int m,const double* x,const double* vec,double* res) {
  int i,j;
  for(i=0;i<n;i++) {
    double tmp = 0;
    for(j=0;j<m;j++) {
      tmp+=x[i*m+j]*vec[j];
    }
    res[i] = tmp;
  }
}

void vec_subtract(int n,const double* v1,const double* v2,double* to) {
  int i;
  for(i=0;i<n;i++) {
    to[i] = v1[i] - v2[i];
  }
}
