// @copyright (c) 2009 CSIRO
// Australia Telescope National Facility (ATNF)
// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
// PO Box 76, Epping NSW 1710, Australia
// atnf-enquiries@csiro.au
//
// This file is part of the ASKAP software distribution.
//
// The ASKAP software distribution is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// @author Ben Humphreys <ben.humphreys@csiro.au>
// @author Tim Cornwell  <tim.cornwell@csiro.au>

// System includes
#include <stdio.h>

// Local includes
#include "CudaGridKernel.h"

// Constants
static const int cg_maxSupport = 256;

// Check and report last error
__host__ __inline__ void checkError(void)
{
        cudaError_t err = cudaGetLastError();
        if (err != cudaSuccess)
        {
                printf("CUDA Error: %s\n", cudaGetErrorString(err));
        }
}

// Perform Gridding (Device Function)
// Each thread handles a different grid point
__global__ void d_gridKernel(const Complex *data, const int support,
		const Complex *C, const int *cOffset,
		const int *iu, const int *iv,
		Complex *grid, const int gSize, const int dind)
{
	// The actual starting grid point
	__shared__ int s_gind;
	// The Convoluton function point from which we offset
	__shared__ int s_cind;

	// Calculate the data index offset for this block
	const int l_dind = dind + blockIdx.y;

	// A copy of the vis data so all threads can read it from shared
	// memory rather than all reading from device memory.
	__shared__ Complex l_data;

	if (threadIdx.x == 0) {
		s_gind = iu[l_dind] + gSize * iv[l_dind] - support;
		s_cind = cOffset[l_dind];
		l_data = data[l_dind];
	}
	__syncthreads();

	// Make a local copy from shared memory
	int gind = s_gind;
	int cind = s_cind;

	// blockIdx.x gives the support location in the v direction
	int sSize = 2 * support + 1;
	gind += gSize * blockIdx.x;
	cind += sSize * blockIdx.x;

	// threadIdx.x gives the support location in the u dirction
    grid[gind+threadIdx.x] = cuCfmaf(l_data, C[cind+threadIdx.x], grid[gind+threadIdx.x]);
}

// Perform Gridding (Host Function)
__host__ void cuda_gridKernel(const Complex  *data, const int dSize, const int support,
		const Complex *C, const int *cOffset,
		const int *iu, const int *iv,
		Complex *grid, const int gSize,
		const int *h_iu, const int *h_iv)
{
    cudaFuncSetCacheConfig(d_gridKernel, cudaFuncCachePreferL1);

	const int sSize=2*support+1;
	int step = 1;

	// This loop begs some explanation. It steps through each spectral
	// sample either one at a time or two at a time. It will do two samples
	// if the two regions involved do not overlap. If they do, only a 
	// single point is gridded.
	//
	// Gridding two point is better than one because giving the GPU more
	// work to do allows it to hide memory latency better.
	for (int dind = 0; dind < dSize; dind += step) {
		if ((dind+1) < dSize && (
		        (h_iu[dind] - h_iu[dind+1]) > sSize ||
		        (h_iv[dind] - h_iv[dind+1]) > sSize)) {
		    step = 2;
		} else {
			step = 1;
		}
   		dim3 gridDim(sSize, step);
		d_gridKernel<<< gridDim, sSize >>>(data, support,
			C, cOffset, iu, iv, grid, gSize, dind);
           	checkError();
	}
}

// Perform De-Gridding (Device Function)
__global__ void d_degridKernel(const Complex *grid, const int gSize, const int support,
                const Complex *C, const int *cOffset,
                const int *iu, const int *iv,
                Complex  *data, const int dind,
		int row)
{
	// Private data for each thread. Eventually summed by the
	// master thread (i.e. threadIdx.x == 0). Currently 
	__shared__ Complex s_data[cg_maxSupport];
	s_data[threadIdx.x] = make_cuFloatComplex(0, 0);

	const int l_dind = dind + blockIdx.x;

        // The actual starting grid point
        __shared__ int s_gind;
        // The Convoluton function point from which we offset
        __shared__ int s_cind;

        if (threadIdx.x == 0) {
                s_gind = iu[l_dind] + gSize * iv[l_dind] - support;
                s_cind = cOffset[l_dind];
        }
        __syncthreads();

        // Make a local copy from shared memory
        int gind = s_gind;
        int cind = s_cind;

        // row gives the support location in the v direction
        int sSize = 2 * support + 1;
        gind += gSize * row;
        cind += sSize * row;

	// threadIdx.x gives the support location in the u dirction
	s_data[threadIdx.x] = cuCmulf(grid[gind+threadIdx.x], C[cind+threadIdx.x]);

	// Sum all the private data elements and accumulate to the
	// device memory
        __syncthreads();
	if (threadIdx.x == 0) {
		Complex sum = make_cuFloatComplex(0, 0);
		Complex original;
		original = data[l_dind];
		#pragma unroll 129
		for (int i = 0; i < sSize; ++i) {
			sum = cuCaddf(sum, s_data[i]);
		}
		original = cuCaddf(original, sum);
		data[l_dind] = original;
	}
}

// Perform De-Gridding (Host Function)
__host__ void cuda_degridKernel(const Complex *grid, const int gSize, const int support,
                const Complex *C, const int *cOffset,
                const int *iu, const int *iv,
                Complex  *data, const int dSize)
{
    cudaFuncSetCacheConfig(d_degridKernel, cudaFuncCachePreferL1);

    int sSize = 2 * support + 1;
	if (sSize > cg_maxSupport) {
		printf("Support size of %d exceeds max support size of %d\n",
			sSize, cg_maxSupport);
	}

	int dimGrid = 4096;	// 4096 is starting size
	for (int dind = 0; dind < dSize; dind += dimGrid) {
		if ((dSize - dind) < dimGrid) {
            // If there are less than 4096 elements left,
            // just do the remaining
            dimGrid = dSize - dind;
        }

        for (int row = 0; row < sSize; ++row) {
            d_degridKernel<<< dimGrid, sSize >>>(grid, gSize, support,
                    C, cOffset, iu, iv, data, dind, row);
            checkError();
        }
    }
}
