/// @file
/// @brief Common functionality for all mosaicing gridders
/// @details AProjectGridderBase class encapsulates common operations for all mosaicing 
/// gridders: CF cache support and recalculation statistics, support for the buffer in the uv-space,
/// and the factory of illumination pattrns.
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
///

#include <gridding/AProjectGridderBase.h>
#include <askap/AskapError.h>

using namespace askap;
using namespace askap::synthesis;

/// @brief initialise common part for mosaicing gridders
/// @param[in] maxFeeds Maximum number of feeds allowed
/// @param[in] maxFields Maximum number of fields allowed
/// @param[in] pointingTol Pointing tolerance in radians
/// @param[in] paTol Parallactic angle tolerance in radians
/// @param[in] freqTol Frequency tolerance (relative, threshold for df/f), negative value 
///        means the frequency axis is ignored 
AProjectGridderBase::AProjectGridderBase(const int maxFeeds, const int maxFields, 
                     const double pointingTol, const double paTol, const double freqTol) :
          itsPointingTolerance(pointingTol),  itsParallacticAngleTolerance(paTol),
          itsLastField(-1), itsCurrentField(0),
          itsDone(maxFeeds, maxFields, false), itsPointings(maxFeeds, maxFields, casa::MVDirection()),
          itsNumberOfCFGenerations(0), itsNumberOfIterations(0), 
          itsNumberOfCFGenerationsDueToPA(0), itsCFParallacticAngle(0),
          itsNumberOfCFGenerationsDueToFreq(0), itsFrequencyTolerance(freqTol)                    
{
  ASKAPCHECK(maxFeeds>0, "Maximum number of feeds must be one or more");
  ASKAPCHECK(maxFields>0, "Maximum number of fields must be one or more");
}

/// @brief set up buffer in the uv-space
/// @details To work with illumination patterns we need a buffer. Moving initialisation
/// out of the loop allows to improve the performance. This method is supposed to be called
/// as soon as all necessary parameters are known.
/// @param[in] uSize size in the direction of u-coordinate
/// @param[in] vSize size in the direction of v-coordinate
/// @param[in] uCellSize size of the uv-cell in the direction of 
///            u-coordinate (in wavelengths)
/// @param[in] vCellSize size of the uv-cell in the direction of 
///            v-coordinate (in wavelengths)
/// @param[in] overSample oversampling factor (default is 1)
void AProjectGridderBase::initUVPattern(casa::uInt uSize, casa::uInt vSize, double uCellSize,
                     double vCellSize, casa::uInt overSample)
{
  itsPattern.reset(new UVPattern(uSize,vSize, uCellSize,vCellSize,overSample));
}

/// @brief checks whether the current field has been updated
/// @details See currentField for more detailed description.
/// @param[in] acc input const accessor to analyse
void AProjectGridderBase::indexField(const IConstDataAccessor &acc)
{
  acc.nRow();
}

/// @brief check whether CF cache is valid
/// @details This methods validates CF cache for one particular iteration. If necessary, 
/// all values in itsDone are set to false. This method also sets some internal flags to
/// update the stats correctly when updateStats is called. 
/// @param[in] acc input const accessor to analyse
void AProjectGridderBase::validateCFCache(const IConstDataAccessor &acc)
{
  acc.nRow();
}

/// @brief update statistics
/// @details This class maintains cache rebuild statistics. It is impossible to update them 
/// directly in validateCFCache because a priori it is not known how many CFs are recalculated
/// following invalidation. It depends on the actual algorithm and the dataset. To keep track
/// of the cache rebuild stats call this method with the exact number of CFs calculated.
/// @param[in] nDone number of convolution functions rebuilt at this iteration
void AProjectGridderBase::updateStats(casa::uInt nDone)
{
  itsNumberOfCFGenerations += nDone;
}

