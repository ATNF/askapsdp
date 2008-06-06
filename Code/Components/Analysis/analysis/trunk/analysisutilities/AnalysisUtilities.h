/// @file
///
/// Provides general utility functions to support the analysis code
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef ASKAP_ANALYSIS_ANALYSISUTILS_H_
#define ASKAP_ANALYSIS_ANALYSISUTILS_H_

#include <string>
#include <vector>

#include <APS/ParameterSet.h>

#include <duchamp/Utils/Section.hh>
#include <duchamp/param.hh>

namespace askap
{
  namespace analysis
  {

    class SubimageDef
    {
    public:
      SubimageDef(){itsNAxis=0; itsNSubX=itsNSubY=itsNSubZ=1;};
      SubimageDef& operator= (const SubimageDef& s);
      virtual ~SubimageDef(){};
      void define(const LOFAR::ACC::APS::ParameterSet& parset);
      duchamp::Section section(int workerNum);
      int numSubs(){return itsNSubX*itsNSubY*itsNSubZ;};
    protected:
      int itsNSubX;
      int itsNSubY;
      int itsNSubZ;
      int itsOverlapX;
      int itsOverlapY;
      int itsOverlapZ;
      int *itsNSub;
      int *itsOverlap;
      int itsNAxis;
      std::string itsImageName;
    };

    /// @brief Return an array of axis dimensions for a FITS file.
    /// @ingroup analysisutilities
    long * getFITSdimensions(std::string filename);

    /// @brief Return the probability of obtaining a chisq value by
    ///        chance, for a certain number of degrees of freedom.
    /// @ingroup analysisutilities
    float chisqProb(float ndof, float chisq);

    /// @brief Parse a ParameterSet and define duchamp::param parameters.
    /// @ingroup analysisutilities
    duchamp::Param parseParset(const LOFAR::ACC::APS::ParameterSet& parset);

    /// @brief Find an rms for an array given a mean value
    /// @ingroup analysisutilities
    double findSpread(bool robust, double middle, int size, float *array);

    /// @brief Find an rms for an array given a mean value, with masking of pixels.
    /// @ingroup analysisutilities
    double findSpread(bool robust, double middle, int size, float *array, bool *mask);

    /// @brief Read in image sections and return a vector of duchamp::Section objects;
    /// @ingroup analysisutilities
    std::vector<duchamp::Section> readSectionInfo(std::string filename);

    /// @brief Return a filename for a subimage
    /// @ingroup analysisutilities
    std::string getSubImageName(std::string image, int rank, int numWorkers);

    /// @brief Return a vector list of subsections, one for each worker.
    /// @ingroup analysisutilities
    std::vector<duchamp::Section> getSectionList(int numWorkers, const LOFAR::ACC::APS::ParameterSet& parset);
    duchamp::Section getSection(int workerNum, const LOFAR::ACC::APS::ParameterSet& parset);
    /// @brief Make subimages and return a vector list of subsections.
    /// @ingroup analysisutilities
    std::vector<duchamp::Section> makeSubImages(int numWorkers, const LOFAR::ACC::APS::ParameterSet& parset);

  }
}

#endif
