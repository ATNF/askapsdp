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

#include "utils/DelayEstimator.h"
#include <casa/BasicSL/Constants.h>
#include <askap/AskapError.h>

#include <vector>

namespace askap {

namespace scimath {

/// @brief construct estimator for a given spectral resolution
/// @param[in] resolution the spectral resolution in Hz
DelayEstimator::DelayEstimator(const double resolution) : itsResolution(resolution) {}
   
/// @brief estimate delay for a given spectrum
/// @param[in] vis (visibility) spectrum
/// @return delay in seconds
double DelayEstimator::getDelay(const casa::Vector<casa::Complex> &vis) const
{
   ASKAPDEBUGASSERT(itsResolution != 0.);
   std::vector<float> phases(vis.nelements());
   const float threshold = 3 * casa::C::pi / 2;

   // unambiguate phases
   float wrapCompensation = 0.;
   for (size_t chan=0; chan<phases.size(); ++chan) {
        const casa::Float curPhase = arg(vis[chan]);
        if (chan > 0) {
            const float prevOrigPhase = phases[chan - 1] - wrapCompensation;
            const float diff = curPhase - prevOrigPhase;
            if (diff >= threshold) {
                wrapCompensation -= 2. * casa::C::pi;
            } else if (diff <= -threshold) {
                wrapCompensation += 2. * casa::C::pi;
            }
        }
        phases[chan] = curPhase + wrapCompensation;
   }
   
   // do LSF into phase vs. channel
   double sx = 0., sy = 0., sx2 = 0., sxy = 0.;
   // could've combined two loops, but keep it easy for now
   for (size_t chan=0; chan < phases.size(); ++chan) {
        sx += double(chan);
        sx2 += double(chan)*double(chan);
        sy += double(phases[chan]);
        sxy += double(chan)*double(phases[chan]);
   }
   sx /= double(phases.size());
   sy /= double(phases.size());
   sx2 /= double(phases.size());
   sxy /= double(phases.size());
   const double coeff = (sxy - sx * sy) / (sx2 - sx * sx);
   // calculate delay based on the fitted slope
   return coeff / 2. / casa::C::pi / itsResolution;
}

} // namespace scimath

} // namespace askap
