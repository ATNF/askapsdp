/// @file
/// 
/// @brief Base class for generic measurement equation for calibration with pre-averaging.
/// @details This is a base class for a template designed to represent any 
/// possible measurement equation we expect to encounter in calibration. 
/// It is similar to CalibrationMEBase, but implements pre-averaging (or pre-summing to be
/// exact) using PreAvgCalBuffer, so that only one iteration over the data is required.
/// Because of this, the method to calculate normal equations without parameters is the one
/// which is supposed to be used.
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

#include <measurementequation/PreAvgCalMEBase.h>
#include <dataaccess/IDataIterator.h>
#include <askap/AskapError.h>
#include <fitting/ComplexDiffMatrix.h>
#include <fitting/ComplexDiff.h>
#include <fitting/DesignMatrix.h>
#include <casa/Arrays/MatrixMath.h>
#include <askap_synthesis.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".measurementequation.preavgcalmebase");



using namespace askap;
using namespace askap::synthesis;

/// @brief constructor setting up only parameters
/// @param[in] ip Parameters
PreAvgCalMEBase::PreAvgCalMEBase(const askap::scimath::Params& ip) :
    scimath::GenericEquation(ip) {}

/// @brief Standard constructor using the parameters and the
/// data iterator.
/// @param[in] ip Parameters
/// @param[in] idi data iterator
/// @param[in] ime measurement equation describing perfect visibilities
/// @note This version does iteration over the dataset and all accumulation.
PreAvgCalMEBase::PreAvgCalMEBase(const askap::scimath::Params& ip,
        const accessors::IDataSharedIter& idi, 
        const boost::shared_ptr<IMeasurementEquation const> &ime) :
        scimath::GenericEquation(ip)
{
  accumulate(idi,ime);
}
          
/// @brief accumulate one accessor
/// @details This method processes one accessor and accumulates the data.
/// It is essentially a proxy for the accumulate method of the buffer.
/// @param[in] acc data accessor
/// @param[in] me measurement equation describing perfect visibilities
void PreAvgCalMEBase::accumulate(const accessors::IConstDataAccessor &acc,  
          const boost::shared_ptr<IMeasurementEquation const> &me)
{
  itsBuffer.accumulate(acc,me);
}
          
/// @brief accumulate all data
/// @details This method iterates over the whole dataset and accumulates all
/// the data.
/// @param[in] idi data iterator
/// @param[in] ime measurement equation describing perfect visibilities
void PreAvgCalMEBase::accumulate(const accessors::IDataSharedIter& idi, 
        const boost::shared_ptr<IMeasurementEquation const> &ime)
{
  accessors::IDataSharedIter iter(idi);
  for (iter.init(); iter.hasMore(); iter.next()) {
       itsBuffer.accumulate(*iter,ime);
  }
}        
                    
/// @brief Predict model visibilities for one accessor (chunk).
/// @details This class cannot be used for prediction 
/// (use CalibrationMEBase instead). Therefore this method just 
/// throws an exception.
void PreAvgCalMEBase::predict() const
{
  ASKAPTHROW(AskapError, "PreAvgCalMEBase::predict() is not supposed to be called");
}

/// @brief calculate normal equations in the general form 
/// @details This method calculates normal equations for the
/// given set of parameters. It is assumed that some data have already 
/// been accumulated.
/// @param[in] ne normal equations to update
void PreAvgCalMEBase::calcGenericEquations(scimath::GenericNormalEquations &ne) const
{  
  for (casa::uInt row = 0; row < itsBuffer.nRow(); ++row) { 
       const casa::Matrix<casa::Float> sumModelAmps = itsBuffer.sumModelAmps().yzPlane(row); 
       const casa::Matrix<casa::Complex> sumVisProducts = itsBuffer.sumVisProducts().yzPlane(row);
       const casa::Matrix<casa::Complex> sumModelProducts = itsBuffer.sumModelProducts().yzPlane(row);
       ASKAPDEBUGASSERT(sumVisProducts.ncolumn() == (itsBuffer.nPol()*(itsBuffer.nPol()+1)/2));
       ASKAPDEBUGASSERT(sumVisProducts.nrow() == itsBuffer.nChannel());
       ASKAPDEBUGASSERT(sumModelProducts.ncolumn() == (itsBuffer.nPol()*(itsBuffer.nPol()-1)/2));
       ASKAPDEBUGASSERT(sumModelProducts.nrow() == itsBuffer.nChannel());

       scimath::ComplexDiffMatrix cdm = buildComplexDiffMatrix(itsBuffer, row); 
       for (casa::uInt chan = 0; chan < itsBuffer.nChannel(); ++chan) {
            casa::Vector<casa::Float> sumModelAmpsVect = sumModelAmps.row(chan);
            ASKAPDEBUGASSERT(sumModelAmpsVect.nelements() == itsBuffer.nPol());
            scimath::ComplexDiffMatrix cdVect(itsBuffer.nPol()*itsBuffer.nPol(),1,0.);
            casa::Vector<casa::Complex> measuredVect(cdVect.nRow()); // size is nPol*nPol (for cross-terms)
            
            // we have nPol x nPol equations (each original design equation is 
            // multiplied by conjugate of Vpol1)
            for (casa::uInt pol1 = 0, eqn = 0; pol1<itsBuffer.nPol(); ++pol1)   {
                 for (casa::uInt pol2 = 0; pol2 < itsBuffer.nPol(); ++pol2, ++eqn) {
                      if (pol1 == pol2) {
                          // parallel hand data term
                          measuredVect[eqn] = sumVisProducts(chan,pol2);
                      } else {
                          // cross terms (equations corresponding to multiplication by another
                          // polarisation product
                          const casa::uInt minPol = casa::min(pol1,pol2);
                          const casa::uInt maxPol = casa::max(pol1,pol2);
                          // index into sumVisProducts
                          const casa::uInt svpIndex = itsBuffer.polToIndex(maxPol,minPol); 
                          ASKAPDEBUGASSERT(svpIndex < sumVisProducts.nchannel());
                          measuredVect[eqn] = pol1 > pol2 ? sumVisProducts(chan,svpIndex) :
                                                            std::conj(sumVisProducts(chan,svpIndex));
                      }
                      // this loop is to form the matrix element
                      for (casa::uInt pol = 0; pol < itsBuffer.nPol(); ++pol) {
                           const casa::uInt minPol = casa::min(pol1,pol);
                           const casa::uInt maxPol = casa::max(pol1,pol);
                           // index into sumVisProducts (we access sumModelProducts only,
                           // but this index takes care of parallel hand elements)
                           const casa::uInt svpIndex = itsBuffer.polToIndex(maxPol,minPol); 
                           if (svpIndex < itsBuffer.nPol()) {
                               // parallel hand case
                               ASKAPDEBUGASSERT(pol1 == pol);
                               // treat amplitudes separately, because they are real, not complex -
                               // no unnecessary equations this way
                               cdVect[eqn] += cdm(pol2,pol) * sumModelAmpsVect[pol];
                           } else {
                               // offset to get index into sumModelProducts
                               const casa::uInt smpIndex = svpIndex - itsBuffer.nPol();
                               ASKAPDEBUGASSERT(smpIndex < sumModelProducts.ncolumn());
                               const casa::Complex smp = pol1 > pol ? sumModelProducts(chan,smpIndex) :
                                                         std::conj(sumModelProducts(chan,smpIndex));
                               cdVect[eqn] += cdm(pol2,pol) * smp;
                           }
                      }
                 }
            }

            scimath::DesignMatrix designmatrix;
            designmatrix.addModel(cdVect, measuredVect, 
                 casa::Vector<double>(measuredVect.nelements(),1.));      
            ne.add(designmatrix);            
       }

       /*
       scimath::ComplexDiffMatrix cdm = buildComplexDiffMatrix(itsBuffer, row) *       
            scimath::ComplexDiffMatrix(sumModelAmps);
       casa::Matrix<casa::Complex> tempMeasured = itsBuffer.sumVisProducts().yzPlane(row);     
       casa::Matrix<casa::Complex> measuredSlice = transpose(tempMeasured(casa::Slice(),
                                                             casa::Slice(0,itsBuffer.nPol())));
       //std::cout<<"row="<<row<<" measuredSlice="<<casa::Vector<casa::Complex>(measuredSlice)<<" "<<cdm(1,0).value()<<std::endl;
       
       scimath::DesignMatrix designmatrix;
       // we can probably add below actual weights taken from the data accessor
       designmatrix.addModel(cdm, measuredSlice, 
                 casa::Matrix<double>(measuredSlice.nrow(),
                 measuredSlice.ncolumn(),1.));      
       ne.add(designmatrix);
       */
  }
}
  
/// @brief initialise accumulation
/// @details Resets the buffer and configure it to the given number of
/// antennas and beams.
/// @param[in] nAnt number of antennas
/// @param[in] nBeam number of beams
void PreAvgCalMEBase::initialise(casa::uInt nAnt, casa::uInt nBeam)
{
  itsBuffer.initialise(nAnt,nBeam);
}

/// @brief destructor 
/// @details This method just prints statistics on the number of
/// visibilities not accumulated due to various reasons
PreAvgCalMEBase::~PreAvgCalMEBase()
{
  ASKAPLOG_DEBUG_STR(logger, "PreAvgCalMEBase statistics on ignored visibilities");
  ASKAPLOG_DEBUG_STR(logger, "   ignored due to type (e.g. autocorrelations): "<<itsBuffer.ignoredDueToType());
  ASKAPLOG_DEBUG_STR(logger, "   no match found for baseline/beam: "<<itsBuffer.ignoredNoMatch());
  ASKAPLOG_DEBUG_STR(logger, "   ignored because of flags: "<<itsBuffer.ignoredDueToFlags());
}


  
