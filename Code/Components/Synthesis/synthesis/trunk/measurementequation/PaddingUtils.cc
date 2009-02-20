/// @file
///
/// PaddingUtils a class containing utilities used for FFT padding in preconditioners. Code like this
/// can probably be moved to a higer level. At this stage we just need to make these methods available not
/// just to the WienerPreconditioner, but for other classes as well.
///
/// @copyright (c) 2007 CSIRO
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#include <measurementequation/PaddingUtils.h>

#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation");

#include <askap/AskapError.h>

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/LatticeExpr.h>
#include <lattices/Lattices/SubLattice.h>

#include <fft/FFTWrapper.h>
#include <casa/Arrays/ArrayIter.h>


using namespace askap;
using namespace synthesis;

/// @brief Inject source into the centre quarter of the target
/// @details 
/// @param[in] target target array to alter, the source will be converted to Complex and stored in the 
/// inner quarter of the target
/// @param[in] source input array
void PaddingUtils::inject(casa::Lattice<casa::Complex>& target, casa::Lattice<float>& source)
{
  target.set(0.0);
  casa::IPosition corner(target.shape().nelements(),0);
  ASKAPDEBUGASSERT(corner.nelements()>=2);
  ASKAPDEBUGASSERT(target.shape()(0) == source.shape()(0)*2);
  ASKAPDEBUGASSERT(target.shape()(1) == source.shape()(1)*2);
      
  corner(0) = target.shape()(0)/4;
  corner(1) = target.shape()(1)/4;
  casa::Slicer slicer(corner, source.shape());
  casa::SubLattice<casa::Complex> inner(target, slicer, casa::True);
  inner.copyData(casa::LatticeExpr<casa::Complex>(toComplex(source)));
  //      ASKAPLOG_INFO_STR(logger, "Injected " << source.shape() << " into " << target.shape() << " starting at " << corner);
}
      
/// @brief Extract target from the center quarter of the source 
/// @details
/// @param[in] target target array to save the reslt, a real part of the inner quarter of the the source array 
/// will be extracted
/// @param[in] source input array
void PaddingUtils::extract(casa::Lattice<float>& target, casa::Lattice<casa::Complex>& source)
{
  target.set(0.0);
  casa::IPosition corner(source.shape().nelements(),0);
  ASKAPDEBUGASSERT(corner.nelements()>=2);
  ASKAPDEBUGASSERT(source.shape()(0) == target.shape()(0)*2);
  ASKAPDEBUGASSERT(source.shape()(1) == target.shape()(1)*2);
  corner(0) = source.shape()(0)/4;
  corner(1) = source.shape()(1)/4;
  casa::Slicer slicer(corner, target.shape());
  casa::SubLattice<casa::Complex> inner(source, slicer, casa::True);
  target.copyData(casa::LatticeExpr<float>(real(inner)));
  //      ASKAPLOG_INFO_STR(logger, "Extracted " << target.shape() << " from " << source.shape() << " starting at " << corner); 
}


/// @brief helper method to get padded shape
/// @details Most padding applications in the ASKAPsoft require operations on just two
/// axes. This method froms a shape of an array padded on first two axes with the given factor.
/// @param[in] shape shape of the original array
/// @param[in] padding padding factor
/// @return shape of the padded array
casa::IPosition PaddingUtils::paddedShape(const casa::IPosition &shape, const int padding)
{
  casa::IPosition result(shape);
  ASKAPDEBUGASSERT(result.nelements()>=2);
  ASKAPDEBUGASSERT(padding>0);
  result(0) *= padding;
  result(1) *= padding;
  return result;
}

/// @brief padding with fft
/// @details Sometimes it is necessary to do padding in the other domain. This routine 
/// does the Fourier transform, pad the result to the size of the output and then transforms 
/// back to the original domain. It is done if the size of the output array along the first two
/// axes is larger than the size of the input array. If the output array size is smaller, just 
/// the inner subimage is copied and no fft is done. Equal size result in no operation.
/// @note Both input and output arrays should be at least 2-dimensional, otherwise an exception
/// is thrown. 
/// @param[in] in input array 
/// @param[in] out output array (should already be resized to a desired size) 
void PaddingUtils::fftPad(const casa::Array<double>& in, casa::Array<double>& out)
{
  ASKAPDEBUGASSERT(in.shape().nelements()>=2);    
  const int inx=in.shape()(0);
  const int iny=in.shape()(1);

  ASKAPDEBUGASSERT(out.shape().nelements()>=2);    
      
  const int onx=out.shape()(0);
  const int ony=out.shape()(1);
            
  // Shortcut no-op
  if ((inx == onx) && (iny == ony)) {
       out = in.copy();
       return;
  }
      
  ASKAPCHECK((onx >= inx) == (ony >= iny), 
             "Attempting to pad to a rectangular array smaller on one axis");
  if (onx<inx) {
      // no fft padding required, the output array is smaller.
      casa::Array<double> tempIn(in); // in is a conceptual const array here
      out = centeredSubArray(tempIn,out.shape()).copy();
      return;
  }
      
      
  /// Make an iterator that returns plane by plane
  casa::ReadOnlyArrayIterator<double> inIt(in, 2);
  casa::ArrayIterator<double> outIt(out, 2);
  while (!inIt.pastEnd()&&!outIt.pastEnd()) {
         casa::Matrix<casa::DComplex> inPlane(inx, iny);
         casa::Matrix<casa::DComplex> outPlane(onx, ony);
         casa::convertArray(inPlane, inIt.array());
         outPlane.set(0.0);
         fft2d(inPlane, false);
         for (int iy=0; iy<iny; ++iy) {
              for (int ix=0; ix<inx; ++ix) {
                   outPlane(ix+(onx-inx)/2, iy+(ony-iny)/2) = inPlane(ix, iy);
              }
         }
         
         fft2d(outPlane, true);
         const casa::Array<casa::DComplex> constOutPlane(outPlane);
         casa::Array<double> outArray(outIt.array());
	
         casa::real(outArray, constOutPlane);
	
         inIt.next();
         outIt.next();
  }
}
