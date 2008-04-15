/// @file
///
/// Provides general utility functions to support the analysis code
///
/// (c) 2007 ASKAP, All Rights Reserved.
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

    /// @brief Read in image sections and return a vector of duchamp::Section objects;
    /// @ingroup analysisutilities
    std::vector<duchamp::Section> readSectionInfo(std::string filename);

    /// @brief Return a filename for a subimage
    /// @ingroup analysisutilities
    std::string getSubImageName(std::string image, int rank, int numWorkers);

    /// @brief Make subimages and return a vector list of subsections.
    /// @ingroup analysisutilities
    std::vector<duchamp::Section> makeSubImages(int numWorkers, const LOFAR::ACC::APS::ParameterSet& parset);

  }
}

#endif
