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
#include <askap/AskapUtil.h>
#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".gridding");


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
          itsNumberOfCFGenerationsDueToFreq(0), itsFrequencyTolerance(freqTol),
          itsCFInvalidDueToPA(false), itsCFInvalidDueToFreq(false)
{
  ASKAPCHECK(maxFeeds>0, "Maximum number of feeds must be one or more");
  ASKAPCHECK(maxFields>0, "Maximum number of fields must be one or more");
}

/// @brief copy constructor
/// @details It is needed because we have a shared pointer as a data member and want to
/// clone the object instead of copying the reference as if it would be by default.
/// @param[in] other input object
AProjectGridderBase::AProjectGridderBase(const AProjectGridderBase &other) : 
    itsPointingTolerance(other.itsPointingTolerance),
    itsParallacticAngleTolerance(other.itsParallacticAngleTolerance),
    itsLastField(other.itsLastField), itsCurrentField(other.itsCurrentField),
    itsDone(other.itsDone.copy()), itsPointings(other.itsPointings.copy()), 
    itsNumberOfCFGenerations(other.itsNumberOfCFGenerations),
    itsNumberOfIterations(other.itsNumberOfIterations),
    itsNumberOfCFGenerationsDueToPA(other.itsNumberOfCFGenerationsDueToPA), 
    itsCFParallacticAngle(other.itsCFParallacticAngle),
    itsNumberOfCFGenerationsDueToFreq(other.itsNumberOfCFGenerationsDueToFreq),
    itsFrequencyTolerance(other.itsFrequencyTolerance),
    itsCachedFrequencies(other.itsCachedFrequencies),
    itsCFInvalidDueToPA(other.itsCFInvalidDueToPA),
    itsCFInvalidDueToFreq(other.itsCFInvalidDueToFreq)
{
  if (other.itsPattern) {
      itsPattern.reset(new UVPattern(*(other.itsPattern)));
  }
}
  
/// @brief destructor
/// @details We print cache usage stats here. No specific destruction is required for any data member
AProjectGridderBase::~AProjectGridderBase()
{
  size_t nUsed = 0;
  for (casa::uInt feed = 0; feed<itsDone.nrow(); ++feed) {
      for (casa::uInt field = 0; field<itsDone.ncolumn(); ++field) {
           if (isCFValid(feed,field)) {
               ++nUsed;
            }
      }
  }  
  if (itsDone.nelements()) {
      ASKAPLOG_INFO_STR(logger, "AProjectGridderBase: CF cache memory utilisation (last iteration): "<<
              double(nUsed)/double(itsDone.nrow()*itsDone.ncolumn())*100<<"% of maxfeed*maxfield");
  }
  
  if (itsNumberOfIterations != 0) {
      ASKAPLOG_INFO_STR(logger, "AProjectGridderBase: CFs were rebuilt "<<
             itsNumberOfCFGenerations<<" times for "<<itsNumberOfIterations<<" iterations");
      ASKAPLOG_INFO_STR(logger, "Last iteration worked with "<<nUsed<<" CFs");        
      if (itsNumberOfCFGenerations != 0) {
          ASKAPLOG_INFO_STR(logger, "Parallactic angle change caused "<<
                  itsNumberOfCFGenerationsDueToPA<<" of those rebuilds ("<<
                  double(itsNumberOfCFGenerationsDueToPA)/double(itsNumberOfCFGenerations)*100<<
                  " %)");
          ASKAPLOG_INFO_STR(logger, "Frequency axis change caused "<<
                  itsNumberOfCFGenerationsDueToFreq<<" of those rebuilds ("<<
                  double(itsNumberOfCFGenerationsDueToFreq)/double(itsNumberOfCFGenerations)*100<<
                  " %)");
      }   
      if (nUsed != 0) { 
          // because nUsed is strictly speaking applicable to the last iteration only we need
          // to filter out rediculous values (and warn the user that the result is approximate
          // anyway)
          const double utilisation = (1.-double(itsNumberOfCFGenerations)/
                                  double(itsNumberOfIterations*nUsed));
          if ((utilisation<1.) && (utilisation>0.)) {
              ASKAPLOG_INFO_STR(logger, "Approximate CF cache utilisation is "<<
                                        utilisation*100.<<" %");
          }
      }
  }
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
  // Validate cache using first row only
  bool newField = true;
  ASKAPDEBUGASSERT(acc.nRow()>0);

  casa::uInt firstFeed = acc.feed1()(0);
  ASKAPCHECK(firstFeed<itsDone.nrow(), "Too many feeds: increase maxfeeds");
  casa::MVDirection firstPointing = acc.pointingDir1()(0);

  for (int field=itsLastField; field>-1; --field) {
       if (firstPointing.separation(pointing(firstFeed, field))<itsPointingTolerance) {
           itsCurrentField = field;
           newField = false;
           break;
       }
  }
  if (newField) {
      ++itsLastField;
      ASKAPDEBUGASSERT(itsLastField>=0);
      itsCurrentField = itsLastField;
      ASKAPCHECK(itsCurrentField < itsDone.ncolumn(),
              "Too many fields: increase maxfields " << itsDone.ncolumn());
      itsPointings(firstFeed, itsCurrentField) = firstPointing;
      ASKAPLOG_INFO_STR(logger, "Found new field " << itsCurrentField<<" at "<<
                printDirection(firstPointing));
  } 
}

/// @brief check whether CF cache is valid
/// @details This methods validates CF cache for one particular iteration. If necessary, 
/// all values in itsDone are set to false. This method also sets some internal flags to
/// update the stats correctly when updateStats is called. 
/// @param[in] acc input const accessor to analyse
/// @param[in] symmetric true, if illumination pattern is symmetric, false otherwise
void AProjectGridderBase::validateCFCache(const IConstDataAccessor &acc, bool symmetric)
{
  const int nSamples = acc.nRow();
 
  // flags are used to accumulate CF rebuild statistics
  itsCFInvalidDueToPA = false;
    
  if (!symmetric) {
      // need to check parallactic angles here
      const casa::Vector<casa::Float> &feed1PAs = acc.feed1PA();
      ASKAPDEBUGASSERT(feed1PAs.nelements() == casa::uInt(nSamples));
      for (int row = 0; row<nSamples; ++row) {
           if (fabs(feed1PAs[row] - itsCFParallacticAngle)<itsParallacticAngleTolerance) {
               itsCFInvalidDueToPA = true;
               itsCFParallacticAngle = feed1PAs[row];
               itsDone.set(false);
               break;
           }
      }
  }
    
  // the following flag is used to accululate CF rebuild statistics and internal logic
  itsCFInvalidDueToFreq = false;
    
  // don't bother checking if the cache is rebuilt anyway
  if (!itsCFInvalidDueToPA && (itsFrequencyTolerance >= 0.)) {
      const casa::Vector<casa::Double> &freq = acc.frequency();
      if (freq.nelements() != itsCachedFrequencies.nelements()) {
          itsCFInvalidDueToFreq = true;
      } else {
          // we can also write the following using iterators, if necessary
          for (casa::uInt chan = 0; chan<freq.nelements(); ++chan) {
               const casa::Double newFreq = freq[chan];
               ASKAPDEBUGASSERT(newFreq > 0.);
               if ( fabs(itsCachedFrequencies[chan] - newFreq)/newFreq > itsFrequencyTolerance) {
                    itsCFInvalidDueToFreq = true;
                    break;
               }
          }
      } 
      if (itsCFInvalidDueToFreq) {
          itsDone.set(false);
      }
  }
    
  // cache the current frequency axis if the cache is going to be built
  // do nothing if the tolerance is negative 
  if ((itsCFInvalidDueToPA || itsCFInvalidDueToFreq) && (itsFrequencyTolerance >= 0.)) {
      itsCachedFrequencies.assign(acc.frequency().copy());
  } 
}

/// @brief update statistics
/// @details This class maintains cache rebuild statistics. It is impossible to update them 
/// directly in validateCFCache because a priori it is not known how many CFs are recalculated
/// following invalidation. It depends on the actual algorithm and the dataset. To keep track
/// of the cache rebuild stats call this method with the exact number of CFs calculated.
/// @param[in] nDone number of convolution functions rebuilt at this iteration
void AProjectGridderBase::updateStats(casa::uInt nDone)
{
  ++itsNumberOfIterations;
  itsNumberOfCFGenerations += nDone;
  if (itsCFInvalidDueToPA) {
      itsNumberOfCFGenerationsDueToPA += nDone;
  }    
  if (itsCFInvalidDueToFreq) {
      itsNumberOfCFGenerationsDueToFreq += nDone;
  }
}

