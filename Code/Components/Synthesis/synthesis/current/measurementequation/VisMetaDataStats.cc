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
VisMetaDataStats::VisMetaDataStats() : itsTangentSet(false), itsAccessorAdapter(-1.), itsNVis(0ul), itsMaxU(0.), 
     itsMaxV(0.), itsMaxW(0.), itsMaxResidualW(0.), itsMinFreq(0.), itsMaxFreq(0.),
     itsMaxAntennaIndex(0u), itsMaxBeamIndex(0u), itsRefDirValid(false), itsFieldBLC(0.,0.), itsFieldTRC(0.,0.) {}

/// @brief constructor with explicitly given tangent point
/// @details We need to know tangent point to estimate the w-term correctly
/// (tangent point is required for uvw-rotation). Unless the tangent point
/// is chosen in advance, a two-pass iteration over the data is required. 
/// The first iteration is used to find out the centre of the field which
/// can be used as a tangent point during imaging. The second pass determines
/// actual stats on the w-term. In the second pass, this class is initialised 
/// with either this version of the constructor or the version specific for 
/// the snap-shot imaging.
/// @param[in] wtolerance threshold triggering fitting of a new plane for snap-shot imaging (wavelengths)      
VisMetaDataStats::VisMetaDataStats(const casa::MVDirection &tangent) : itsTangent(tangent), itsTangentSet(true), itsAccessorAdapter(-1.),
     itsNVis(0ul), itsMaxU(0.), itsMaxV(0.), itsMaxW(0.), itsMaxResidualW(0.), itsMinFreq(0.), itsMaxFreq(0.), 
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
VisMetaDataStats::VisMetaDataStats(const casa::MVDirection &tangent, double wtolerance) : itsTangent(tangent), itsTangentSet(true), 
     itsAccessorAdapter(wtolerance), 
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
  // for now ignore flagging. Technically, some metadata may be ignored if all corresponding data are flagged, but
  // it seems to be too much of the complication now. 
  const double currentMaxFreq = casa::max(acc.frequency());
  const double currentMinFreq = casa::min(acc.frequency());
  const casa::uInt currentMaxAntennaIndex = casa::max(casa::max(acc.antenna1()), casa::max(acc.antenna2()));
  const casa::uInt currentMaxBeamIndex = casa::max(casa::max(acc.feed1()), casa::max(acc.feed2()));
  
  if (itsNVis == 0ul) {
      itsMinFreq = currentMinFreq;
      itsMaxFreq = currentMaxFreq;
      itsMaxAntennaIndex = currentMaxAntennaIndex;
      itsMaxBeamIndex = currentMaxBeamIndex;
  } else {
      if (itsMinFreq > currentMinFreq) {
          itsMinFreq = currentMinFreq;
      }
      if (itsMaxFreq < currentMaxFreq) {
          itsMaxFreq = currentMaxFreq;
      }
      if (itsMaxAntennaIndex < currentMaxAntennaIndex) {
          itsMaxAntennaIndex = currentMaxAntennaIndex;
      }
      if (itsMaxBeamIndex < currentMaxBeamIndex) {
          itsMaxBeamIndex = currentMaxBeamIndex;
      }
  }

  const double reciprocalToShortestWavelength = currentMaxFreq / casa::C::c;
  
  if (itsAccessorAdapter.tolerance() >=0.) {
      ASKAPCHECK(itsTangentSet, "wtolerance has to be set together with the tangent point!")
  } 
  
  if (itsTangentSet) {
      const casa::Vector<casa::RigidVector<casa::Double, 3> > &origUVW = acc.rotatedUVW(itsTangent);
      
      if (itsAccessorAdapter.tolerance() >= 0.) {
          itsAccessorAdapter.associate(acc);
          ASKAPDEBUGASSERT(acc.nRow() == itsAccessorAdapter.nRow());
      }
                     
      for (casa::uInt row=0; row < acc.nRow(); ++row) {
           const double currentU = origUVW[row](0) * reciprocalToShortestWavelength;
           const double currentV = origUVW[row](1) * reciprocalToShortestWavelength;
           const double currentW = origUVW[row](2) * reciprocalToShortestWavelength;
           
           if ((itsNVis == 0ul) && (row == 0)) {
               itsMaxU = currentU;
               itsMaxV = currentV;
               itsMaxW = currentW;
           } else {
               if (itsMaxU < currentU) {
                   itsMaxU = currentU;
               }
               if (itsMaxV < currentV) {
                   itsMaxV = currentV;
               }
               if (itsMaxW < currentW) {
                   itsMaxW = currentW;
               }               
           }
      } 
      if (itsAccessorAdapter.tolerance() >= 0.) {
          const casa::Vector<casa::RigidVector<casa::Double, 3> > &uvw = itsAccessorAdapter.rotatedUVW(itsTangent);
          for (casa::uInt row=0; row < itsAccessorAdapter.nRow(); ++row) {
               const double currentResidualW = uvw[row](2) * reciprocalToShortestWavelength;
               if ((itsNVis == 0ul) && (row == 0)) {
                   itsMaxResidualW = currentResidualW;
               } else {
                   if (itsMaxResidualW < currentResidualW) {
                       itsMaxResidualW = currentResidualW;
                   }               
               }               
          }      
          itsAccessorAdapter.detach();
      }
  } else {
      // this is the first pass, do the best effort job as exact tangent point is unknown
      const casa::Vector<casa::RigidVector<casa::Double, 3> > &uvw = acc.uvw();
      for (casa::uInt row=0; row < acc.nRow(); ++row) {
           const double currentU = uvw[row](0) * reciprocalToShortestWavelength;
           const double currentV = uvw[row](1) * reciprocalToShortestWavelength;
           const double currentW = uvw[row](2) * reciprocalToShortestWavelength;
           if ((itsNVis == 0ul) && (row == 0)) {
               itsMaxU = currentU;
               itsMaxV = currentV;
               itsMaxW = currentW;
           } else {
               if (itsMaxU < currentU) {
                   itsMaxU = currentU;
               }
               if (itsMaxV < currentV) {
                   itsMaxV = currentV;
               }
               if (itsMaxW < currentW) {
                   itsMaxW = currentW;
               }
           }
      }
  }

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

