/// @file
/// 
/// @brief measurement equation to apply calibration.
/// @details This is a special type of the measurement equation (i.e. it is not
/// even derived from the scimath::Equation class because it is not solvable). It
/// corrects a chunk of visibilities for calibration, leakages and bandpasses
/// obtained via the solution access interface. Unlike CalibrationMEBase and
/// PreAvgCalMEBase this class has the full measurement equation built in 
/// (essentially implemented by the solution access class returning a complete
/// jones matrix for each antenna/beam combination).
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

#include <measurementequation/CalibrationApplicatorME.h>
#include <casa/Arrays/MatrixMath.h>
#include <scimath/Mathematics/MatrixMathLA.h>
#include <scimath/Mathematics/RigidVector.h>
#include <askap/AskapError.h>
#include <utils/PolConverter.h>

namespace askap {

namespace synthesis {

/// @brief constructor 
/// @details It initialises ME for a given solution source.
/// @param[in] src calibration solution source to work with
CalibrationApplicatorME::CalibrationApplicatorME(const boost::shared_ptr<accessors::ICalSolutionConstSource> &src) :
     CalibrationSolutionHandler(src) {}

/// @brief correct model visibilities for one accessor (chunk).
/// @details This method corrects the data in the given accessor
/// (accessed via rwVisibility) for the calibration errors 
/// represented by this measurement equation (i.e. an inversion of
/// the matrix has been performed). 
/// @param[in] chunk a read-write accessor to work with
/// @note Need to think what to do in the inversion is unsuccessful
/// e.g. amend flagging information? This is not yet implemented as
/// existing accessors would throw an exception if flagging info is 
/// changed.
void CalibrationApplicatorME::correct(accessors::IDataAccessor &chunk) const
{
  casa::Cube<casa::Complex> &rwVis = chunk.rwVisibility();
  ASKAPDEBUGASSERT(rwVis.nelements());
  updateAccessor(chunk.time());
  const casa::Vector<casa::uInt>& antenna1 = chunk.antenna1();
  const casa::Vector<casa::uInt>& antenna2 = chunk.antenna2();
  const casa::Vector<casa::uInt>& beam1 = chunk.feed1();
  const casa::Vector<casa::uInt>& beam2 = chunk.feed2();
  const casa::Vector<casa::Stokes::StokesTypes> stokes = chunk.stokes();   
  
  const casa::uInt nPol = chunk.nPol();
  ASKAPDEBUGASSERT(nPol <= 4);
  casa::Matrix<casa::Complex> mueller(nPol, nPol);
  casa::Matrix<casa::Complex> reciprocal(nPol, nPol);
  
  casa::RigidVector<casa::uInt, 4> indices(0u);
  for (casa::uInt pol = 0; pol<nPol; ++pol) {
       indices(pol) = scimath::PolConverter::getIndex(stokes[pol]);
  }
  
  // full 4x4 Mueller matrix
  casa::SquareMatrix<casa::Complex, 2> fullMueller(casa::SquareMatrix<casa::Complex, 2>::General);
  
  for (casa::uInt row = 0; row < chunk.nRow(); ++row) {
       casa::Matrix<casa::Complex> thisRow = rwVis.yzPlane(row);
       for (casa::uInt chan = 0; chan < chunk.nChannel(); ++chan) {
            casa::SquareMatrix<casa::Complex, 2> jones1 = calSolution().jones(antenna1[row],beam1[row], chan);
            casa::SquareMatrix<casa::Complex, 2> jones2 = calSolution().jones(antenna2[row],beam2[row], chan);
            for (casa::uInt i = 0; i < nPol; ++i) {
                 for (casa::uInt j = 0; j < nPol; ++j) {
                      const casa::uInt index1 = indices(i);
                      const casa::uInt index2 = indices(j);
                      mueller(i,j) = jones1(index1 / 2, index2 / 2) * conj(jones2(index1 % 2, index2 % 2));
                 }
            }
            
            casa::Complex det = 0.;
            invert(reciprocal, det, mueller);
            
            ASKAPCHECK(casa::abs(det)>1e-5, "Unable to apply calibration, determinate is too close to 0. D="<<casa::abs(det));           
            casa::Vector<casa::Complex> thisChan = thisRow.row(chan);
            const casa::Vector<casa::Complex> origVis = thisChan.copy();
            ASKAPDEBUGASSERT(thisChan.nelements() == nPol);
            // matrix multiplication
            for (casa::uInt pol = 0; pol < nPol; ++pol) {
                 casa::Complex temp(0.,0.);
                 for (casa::uInt k = 0; k < nPol; ++k) {
                     temp += reciprocal(pol,k) * origVis[k];
                 }
                 thisChan[pol] = temp;
            }
       }       
  }
}


} // namespace synthesis

} // namespace askap

