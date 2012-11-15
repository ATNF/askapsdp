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

namespace askap {

namespace synthesis {

/// @brief constructor, initialise class 
VisMetaDataStats::VisMetaDataStats() {}
   
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
  acc.nRow();
}

/// @brief total number of visibility points processed
/// @details This method counts all visibility points. One spectral channel is one
/// visibility point (but polarisations are not counted separately).
/// @return number of visibility points processed so far
unsigned long VisMetaDataStats::nVis() const
{
  return 0ul;
}

   
/// @brief longest baseline spacing in wavelengths
/// @return largest absolute value of u in wavelengths
double VisMetaDataStats::maxU() const { return 0.;}
   
/// @brief longest baseline spacing in wavelengths
/// @return largest absolute value of v in wavelengths
double VisMetaDataStats::maxV() const { return 0.;}
      
/// @brief largest w-term without snap-shotting 
/// @return largest absolute value of w in wavelengths
double VisMetaDataStats::maxW() const {return 0.;}


} // namespace synthesis

} // namespace askap

