/// @file
///
/// @brief An estimator of statistics for metadata associated with visibilities
/// @details Some configuration parameters depend on the metadata, for example
/// cell size depends on the largest baseline. The ASKAP approach is to set
/// all parameters like this a priori to avoid an additional iteration over data.
/// For BETA we could afford iteration over the dataset and, therefore, an "advise"
/// utility could be written. This class handles basic statistics to assist with this.
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
/// @author Max Voronkov <maxim.voronkov@csiro.au
///

#include <measurementequation/VisMetaDataStats.h>
#include <askap/AskapError.h>

namespace askap {

namespace synthesis {

/// @brief constructor, initialise class 
VisMetaDataStats::VisMetaDataStats() : itsAccessorAdapter(-1.), itsNVis(0ul), itsMaxU(0.), 
     itsMaxV(0.), itsMaxW(0.), itsMaxResidualW(0.),
     itsMaxAntennaIndex(0u), itsMaxBeamIndex(0u), itsRefDirValid(false), itsFieldBLC(0.,0.), itsFieldTRC(0.,0.) {}
   
/// @brief constructor specific to snap-shot imaging
/// @details For the snap-shot imaging we need to do two passes unless the desired tangent point
/// can be specified up front. The first pass can be used to find out the centre of the field
/// which can be used as a tangent point during imaging. The second pass, where the class is setup
/// with this version of the constructor, can determine the largest residual w-term for the 
/// given tangent point and w-tolerance.
/// @note For a coplanar array the largest residual w-term will always be less than the w-tolerance
/// which is a threshold for the fitting of a new plane. For non-coplanar array it is not always the
/// case. This is why a complex two-pass estimation procedure is required.
/// @param[in] tangent tangent point to be used with snap-shot imaging (for uvw-rotation)
/// @param[in] wtolerance threshold triggering fitting of a new plane for snap-shot imaging (wavelengths)      
VisMetaDataStats::VisMetaDataStats(const casa::MVDirection &tangent, double wtolerance) : itsTangent(tangent), itsAccessorAdapter(wtolerance), 
     itsNVis(0ul), itsMaxU(0.), itsMaxV(0.), itsMaxW(0.), itsMaxResidualW(0.),
     itsMaxAntennaIndex(0u), itsMaxBeamIndex(0u), itsReferenceDir(tangent), itsRefDirValid(true), itsFieldBLC(0.,0.), itsFieldTRC(0.,0.)
     {}
      
   
/// @brief aggregate statistics with that accumulated by another instance
/// @details This class will be run in parallel if the measurement set is distributed. 
/// This method is intended to combine statistics as part of reduction.
/// @param[in] other an instance of the estimator to take data from
void VisMetaDataStats::merge(const VisMetaDataStats &other)
{
   if (other.nVis() != 0ul) {
   }
}
   
/// @brief process one accessor of data updating statistics
/// @details 
/// @param[in] acc read-only accessor with data
void VisMetaDataStats::process(const accessors::IConstDataAccessor &acc)
{
  itsNVis += acc.nRow() * acc.nChannel();
}

         
/// @brief largest residual w-term (for snap-shotting)
/// @return largest value of residual w in wavelengths
double VisMetaDataStats::maxResidualW() const 
{
  ASKAPCHECK(itsAccessorAdapter.tolerance()>=0., "maxResidualW() called for an object not configured for snap-shot imaging");
  return itsMaxResidualW;
}

/// @brief most central direction of the observed field
/// @return direction of the centre in the frame used by the accessor
casa::MVDirection VisMetaDataStats::centre() const {
  ASKAPCHECK(itsRefDirValid, "centre() called before any visibility has been processed, nvis="<<nVis());
  const std::pair<double,double>  cnt((itsFieldTRC.first + itsFieldBLC.first) / 2, (itsFieldTRC.second + itsFieldBLC.second) / 2);
  casa::MVDirection result(itsReferenceDir);
  result.shift(cnt.first,cnt.second,casa::True);
  return result;
}
   
/// @brief largest separation of individual pointing from the centre
/// @return largest offsets from the centre() in radians (measure of the field size)
std::pair<double,double> VisMetaDataStats::maxOffsets() const 
{
  ASKAPCHECK(itsRefDirValid, "maxOffset() called before any visibility has been processed, nvis="<<nVis());
  const std::pair<double,double>  result((itsFieldTRC.first - itsFieldBLC.first) / 2, (itsFieldTRC.second - itsFieldBLC.second) / 2);
  ASKAPDEBUGASSERT(result.first >= 0.);
  ASKAPDEBUGASSERT(result.second >= 0.);
  return result;
}  


} // namespace synthesis

} // namespace askap

