///
/// GridKernel: kernels for gridding and degridding
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef GRIDKERNEL_H_
#define GRIDKERNEL_H_

#include <casa/aips.h>
#include <casa/Arrays/Matrix.h>
#include <casa/BasicSL/Complex.h>

#include <string>

namespace conrad
{
  namespace synthesis
  {
    /// @brief Holder for gridding kernels
    ///
    /// @ingroup gridding
    class GridKernel
    {
  public:
      /// Information about gridding options
      static std::string info();

      /// Gridding kernel
      static void grid(casa::Matrix<casa::Complex>& grid,
          casa::Complex& sumwt, casa::Matrix<casa::Complex>& convFunc,
          const casa::Complex& cVis, const float& wtVis, const int iu,
          const int iv, const int support);

      /// Degridding kernel
      static void degrid(casa::Complex& cVis,
          const casa::Matrix<casa::Complex>& convFunc,
          const casa::Matrix<casa::Complex>& grid, const int iu, const int iv,
          const int support);

    };
  }
}
#endif
