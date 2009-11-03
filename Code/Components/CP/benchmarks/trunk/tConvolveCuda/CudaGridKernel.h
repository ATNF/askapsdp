#ifndef CUDA_GRID_KERNEL_H
#define CUDA_GRID_KERNEL_H

#include <cuComplex.h>

struct MyComplex {
	float real;
	float imag;
};

typedef cuComplex Complex;

__host__ void cuda_gridKernel(const Complex  *data, const int dSize, const int support,
		const Complex *C, const int *cOffset,
		const int *iu, const int *iv,
		Complex *grid, const int gSize,
		const int *h_iu, const int *h_iv);

__host__ void cuda_degridKernel(const Complex *grid, const int gSize, const int support,
                const Complex *C, const int *cOffset,
                const int *iu, const int *iv,
                Complex  *data, const int dSize);

#endif
