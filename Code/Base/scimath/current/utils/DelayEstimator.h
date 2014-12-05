/// @file
/// 
/// @brief estimate delay from the complex spectrum 
/// @details This class implements a simple algorithm to estimate delay from a complex spectrum
/// by unwrapping the phase and fitting a straight line into the phase slope. This code was
/// originally located in the data monitor of the software correlator, but was moved here so
/// we can reuse it in the CP ingest pipeline.  
///
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

#ifndef ASKAP_SCIMATH_UTILS_DELAY_ESTIMATOR_H
#define ASKAP_SCIMATH_UTILS_DELAY_ESTIMATOR_H

// casa includes
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>


namespace askap {

namespace scimath {

/// @brief estimate delay from the complex spectrum 
/// @details This class implements a simple algorithm to estimate delay from a complex spectrum
/// by unwrapping the phase and fitting a straight line into the phase slope. This code was
/// originally located in the data monitor of the software correlator, but was moved here so
/// we can reuse it in the CP ingest pipeline.  
/// @ingroup utils
class DelayEstimator {
public:
   /// @brief construct estimator for a given spectral resolution
   /// @param[in] resolution the spectral resolution in Hz
   explicit DelayEstimator(const double resolution);
   
   /// @brief estimate delay for a given spectrum
   /// @param[in] vis (visibility) spectrum
   /// @return delay in seconds
   double getDelay(const casa::Vector<casa::Complex> &vis) const;

   /// @brief estimate delay for a given spectrum via FFT
   /// @details This method works well in the case of multiple harmonics
   /// present. However, it only gives a rough estimate
   /// @param[in] vis (visibility) spectrum
   /// @return delay in seconds
   double getDelayWithFFT(const casa::Vector<casa::Complex> &vis) const;

   /// @brief set new spectral resolution
   /// @details The new value will apply to all subsequent calculations
   /// @param[in] resolution the spectral resolution in Hz
   void setResolution(const double resolution);
   
   /// @brief obtain the quality of the latest solution
   /// @details The quality is characterised by a number from 0 to 1, with 1 being the perfect solution and 0
   /// corresponding to failed solution. Exact meaning depends on the method used (and the value is non-linear) 
   inline double quality() const { return itsQuality;}
   
private:
   /// @brief spectral resolution
   double itsResolution;
   
   /// @brief quality metric 
   /// @details The number ranges from 0 to 1 and represents the quality of the latest solution.
   /// The value of 1 corresponds to perfect solution, 0 corresponds to a junk solution. 
   /// It is non-linear and exact meaning depends on the method used. 
   mutable double itsQuality;
   
}; // class DelayEstimator

} // namespace scimath

} // namespace askap

#endif // #ifndef ASKAP_SCIMATH_UTILS_DELAY_ESTIMATOR_H


