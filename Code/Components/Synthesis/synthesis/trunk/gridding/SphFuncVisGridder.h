/// @file
///
/// SphFuncVisGridder: SphFunc-based visibility gridder.
///
/// This supports gridders with a table loopkup.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef SPHVISGRIDDER_H_
#define SPHVISGRIDDER_H_

#include <gridding/TableVisGridder.h>

namespace conrad
{
  namespace synthesis
  {

    class SphFuncVisGridder : public TableVisGridder
    {
      public:

// Standard two dimensional gridding
        SphFuncVisGridder();

        virtual ~SphFuncVisGridder();

/// Correct for gridding convolution function
/// @param axes axes specifications
/// @param image image to be corrected
        virtual void correctConvolution(const scimath::Axes& axes,
          casa::Cube<double>& image);

/// Apply gridding convolution function in image space
/// @param axes axes specifications
/// @param image image to be corrected
        virtual void applyConvolution(const scimath::Axes& axes,
          casa::Cube<double>& image);

      protected:
        virtual int cOffset(int, int);
        virtual void initConvolutionFunction(IDataSharedIter& idi, const casa::Vector<double>& cellSize,
          const casa::IPosition& shape);
      private:
        double grdsf(double nu);
    };

  }
}
#endif
