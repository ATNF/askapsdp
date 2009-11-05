/// @copyright (c) 2009 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///

// Include own header file first
#include "CudaGridder.h"

// System includes
#include <vector>
#include <complex>

// Cuda includes
#include <cuda_runtime_api.h>

// Local includes
#include "CudaGridKernel.h"
#include "Stopwatch.h"

typedef float Real;
typedef std::complex<Real> Value;

void checkerror(cudaError_t err)
{
    if (err != cudaSuccess)
    {
        printf("CUDA Error: %s\n", cudaGetErrorString(err));
    }
}

void gridKernelCuda(const std::vector< std::complex<float> >& data, const int support,
        const std::vector< std::complex<float> >& C, const std::vector<int>& cOffset,
        const std::vector<int>& iu, const std::vector<int>& iv,
        std::vector< std::complex<float> >& grid, const int gSize,
        double &time)
{
    // Need to convert all std::vectors to C arrays for CUDA, then call
    // the kernel exec function. NOTE: The std::vector is the only STL
    // container which you can treat as an array like we do here.

    // Allocate device memory
    Value *d_grid;
    Value *d_C;
    int *d_cOffset;
    int *d_iu;
    int *d_iv;
    Value *d_data;

    cudaError_t err;
    err = cudaMalloc((void **) &d_grid, grid.size() * sizeof(Value));
    checkerror(err);
    err = cudaMalloc((void **) &d_C, C.size() * sizeof(Value));
    checkerror(err);
    err = cudaMalloc((void **) &d_cOffset, cOffset.size() * sizeof(unsigned int));
    checkerror(err);
    err = cudaMalloc((void **) &d_iu, iu.size() * sizeof(unsigned int));
    checkerror(err);
    err = cudaMalloc((void **) &d_iv, iv.size() * sizeof(unsigned int));
    checkerror(err);
    err = cudaMalloc((void **) &d_data, data.size() * sizeof(Value));
    checkerror(err);

    // Copy host vectors to device arrays
    err = cudaMemcpy(d_grid, &grid[0], grid.size() * sizeof(Value), cudaMemcpyHostToDevice);
    checkerror(err);
    err = cudaMemcpy(d_C, &C[0], C.size() * sizeof(Value), cudaMemcpyHostToDevice);
    checkerror(err);
    err = cudaMemcpy(d_cOffset, &cOffset[0], cOffset.size() * sizeof(unsigned int), cudaMemcpyHostToDevice);
    checkerror(err);
    err = cudaMemcpy(d_iu, &iu[0], iu.size() * sizeof(unsigned int), cudaMemcpyHostToDevice);
    checkerror(err);
    err = cudaMemcpy(d_iv, &iv[0], iu.size() * sizeof(unsigned int), cudaMemcpyHostToDevice);
    checkerror(err);
    err = cudaMemcpy(d_data, &data[0], data.size() * sizeof(Value), cudaMemcpyHostToDevice);
    checkerror(err);

    Stopwatch sw;
    sw.start();
    cuda_gridKernel((const Complex *)d_data, data.size(), support,
            (const Complex *)d_C, d_cOffset, d_iu, d_iv,
            (Complex *)d_grid, gSize,
            &iu[0], &iv[0]);
    cudaThreadSynchronize();
    time = sw.stop();

    // Copy device arrays back into the host vector
    err = cudaMemcpy(&grid[0], d_grid, grid.size() * sizeof(Value), cudaMemcpyDeviceToHost);
    checkerror(err);

    // Free device memory
    cudaFree(d_grid);
    cudaFree(d_C);
    cudaFree(d_cOffset);
    cudaFree(d_iu);
    cudaFree(d_iv);
    cudaFree(d_data);
}

void degridKernelCuda(const std::vector< std::complex<float> >& grid,
        const int gSize,
        const int support,
        const std::vector< std::complex<float> >& C,
        const std::vector<int>& cOffset,
        const std::vector<int>& iu,
        const std::vector<int>& iv,
        std::vector< std::complex<float> >& data,
        double &time)
{
    // Need to convert all std::vectors to C arrays for CUDA, then call
    // the kernel exec function. NOTE: The std::vector is the only STL
    // container which you can treat as an array like we do here.

    // Allocate device memory
    Value *d_grid;
    Value *d_C;
    int *d_cOffset;
    int *d_iu;
    int *d_iv;
    Value *d_data;

    cudaError_t err;
    err = cudaMalloc((void **) &d_grid, grid.size() * sizeof(Value));
    checkerror(err);
    err = cudaMalloc((void **) &d_C, C.size() * sizeof(Value));
    checkerror(err);
    err = cudaMalloc((void **) &d_cOffset, cOffset.size() * sizeof(unsigned int));
    checkerror(err);
    err = cudaMalloc((void **) &d_iu, iu.size() * sizeof(unsigned int));
    checkerror(err);
    err = cudaMalloc((void **) &d_iv, iv.size() * sizeof(unsigned int));
    checkerror(err);
    err = cudaMalloc((void **) &d_data, data.size() * sizeof(Value));
    checkerror(err);

    // Copy host vectors to device arrays
    err = cudaMemcpy(d_grid, &grid[0], grid.size() * sizeof(Value), cudaMemcpyHostToDevice);
    checkerror(err);
    err = cudaMemcpy(d_C, &C[0], C.size() * sizeof(Value), cudaMemcpyHostToDevice);
    checkerror(err);
    err = cudaMemcpy(d_cOffset, &cOffset[0], cOffset.size() * sizeof(unsigned int), cudaMemcpyHostToDevice);
    checkerror(err);
    err = cudaMemcpy(d_iu, &iu[0], iu.size() * sizeof(unsigned int), cudaMemcpyHostToDevice);
    checkerror(err);
    err = cudaMemcpy(d_iv, &iv[0], iv.size() * sizeof(unsigned int), cudaMemcpyHostToDevice);
    checkerror(err);
    err = cudaMemcpy(d_data, &data[0], data.size() * sizeof(Value), cudaMemcpyHostToDevice);
    checkerror(err);

    Stopwatch sw;
    sw.start();
    cuda_degridKernel((const Complex *)d_grid, gSize, support,
            (const Complex *)d_C, d_cOffset, d_iu, d_iv,
            (Complex *)d_data, data.size());
    cudaThreadSynchronize();
    time = sw.stop();

    // Copy device arrays back into the host vector
    err = cudaMemcpy(&data[0], d_data, data.size() * sizeof(Value), cudaMemcpyDeviceToHost);
    checkerror(err);

    // Free device memory
    cudaFree(d_grid);
    cudaFree(d_C);
    cudaFree(d_cOffset);
    cudaFree(d_iu);
    cudaFree(d_iv);
    cudaFree(d_data);
}

