#include "GridKernel.h"

/// Use pointers instead of casa::Matrix operators to grid
#define ASKAP_GRID_WITH_POINTERS 1

//#include <askap/AskapError.h>

/// Use BLAS 
/// @todo Ensure that BLAS gridding works on all platforms
///#define ASKAP_GRID_WITH_BLAS 1

#ifdef ASKAP_GRID_WITH_BLAS
#ifdef __APPLE_CC__
#include <vecLib/cblas.h>
#else
#include <cblas.h>
#endif
#endif

namespace askap
{
  namespace synthesis
  {

    std::string GridKernel::info() {
#ifdef ASKAP_GRID_WITH_BLAS
      return std::string("Gridding with BLAS");
#else
#ifdef ASKAP_GRID_WITH_POINTERS
      return std::string("Gridding with casa::Matrix pointers");
#else
      return std::string("Standard gridding/degridding with casa::Matrix");
#endif
#endif
    }
    
    /// Totally selfcontained gridding
    void GridKernel::grid(casa::Matrix<casa::Complex>& grid,
				     casa::Complex& sumwt, casa::Matrix<casa::Complex>& convFunc,
				     const casa::Complex& cVis, const float& viswt, const int iu,
				     const int iv, const int support)
    {
      
#ifdef ASKAP_GRID_WITH_POINTERS || ASKAP_GRID_WITH_BLAS
      for (int suppv=-support; suppv<+support; suppv++)
	{
	  int voff=suppv+support;
	  int uoff=-support+support;
	  casa::Complex *wtPtr=&convFunc(uoff, voff);
	  casa::Complex *gridPtr=&(grid(iu-support, iv+suppv));
#ifdef ASKAP_GRID_WITH_BLAS
          cblas_caxpy(2*support+1, &cVis, wtPtr, 1, gridPtr, 1);
#else
	  for (int suppu=-support; suppu<+support; suppu++)
	    {
	      (*gridPtr)+=cVis*(*wtPtr);
	      wtPtr+=1;
	      gridPtr++;
	    }
#endif
	}
#else
      for (int suppv=-support; suppv<+support; suppv++)
	{
	  int voff=suppv+support;
	  for (int suppu=-support; suppu<+support; suppu++)
	    {
	      int uoff=suppu+support;
	      casa::Complex wt=convFunc(uoff, voff);
	      grid(iu+suppu, iv+suppv)+=cVis*wt;
	    }
	}
#endif
      sumwt+=viswt;
    }
    
    /// Totally selfcontained degridding
    void GridKernel::degrid(casa::Complex& cVis,
				       const casa::Matrix<casa::Complex>& convFunc,
				       const casa::Matrix<casa::Complex>& grid, const int iu, const int iv,
				       const int support)
    {  
      /// Degridding from grid to visibility. Here we just take a weighted sum of the visibility
      /// data using the convolution function as the weighting function. 
      cVis=0.0;
#ifdef ASKAP_GRID_WITH_POINTERS
      for (int suppv=-support; suppv<+support; suppv++)
	{
	  int voff=suppv+support;
	  int uoff=-support+support;
	  const casa::Complex *wtPtr=&convFunc(uoff, voff);
	  const casa::Complex *gridPtr=&(grid(iu-support, iv+suppv));
          casa::Complex dot;
	  for (int suppu=-support; suppu<+support; suppu++)
	    {
	      cVis+=(*wtPtr)*conj(*gridPtr);
	      wtPtr+=1;
	      gridPtr++;
	    }
	}
#else
      for (int suppv=-support; suppv<+support; suppv++)
	{
	  int voff=suppv+support;
	  for (int suppu=-support; suppu<+support; suppu++)
	    {
	      int uoff=suppu+support;
	      casa::Complex wt=convFunc(uoff, voff);
	      cVis+=wt*conj(grid(iu+suppu, iv+suppv));
	    }
	}
#endif
    }
    
  }
}
