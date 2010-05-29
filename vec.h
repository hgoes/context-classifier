/**
 * @file
 * @addtogroup utils Utilities
 * Defines simple, unoptimized matrix/vector operations.
 * @{
 */
#ifndef __VEC_H__
#define __VEC_H__

/**
 * Multiplies a vector with another vector.
 * Both vectors must have the same size.
 * @param n The length of both vectors
 * @param x Vector 1
 * @param y Vector 2
 * @return a scalar
 */
double mult_vec_vec(int n,const double* x,const double* y);

/**
 * Multiply a vector on the right side of a matrix.
 * @param x A matrix of size n*m
 * @param vec A vector of size m
 * @param res The resulting vector of size n
 */
void mult_mat_vec(int n,int m,const double* x,const double* vec,double* res);

/**
 * Subtract two vectors. Both vectors must have same length.
 * @param n The length of both vectors
 * @param v1 Vector 1
 * @param v2 Vector 2
 * @param to The resulting vector of length n
 */
void vec_subtract(int n,const double* v1,const double* v2,double* to);

#endif
/**
 * @}
 */
