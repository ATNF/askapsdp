/// @file
///
/// Provides general utility functions to support the analysis code
///
/// (c) 2007 CONRAD, All Rights Reserved.
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef CONRAD_CP_ANALYSISUTILS_H_
#define CONRAD_CP_ANALYSISUTILS_H_

#include <string>
#include <vector>

#include <APS/ParameterSet.h>

#include <duchamp/Utils/Section.hh>
#include <duchamp/param.hh>

namespace conrad
{
  namespace analysis
  {

    /// @brief Parse a ParameterSet and define duchamp::param parameters.
    /// @ingroup analysisutilities
    duchamp::Param parseParset(const LOFAR::ACC::APS::ParameterSet& parset);

    /// @brief Find an rms for an array given a mean value
    /// @ingroup analysisutilities
    double findSpread(bool robust, double middle, int size, float *array);

    /// @brief Read in image sections and return a vector of duchamp::Section objects;
    /// @ingroup analysisutilities
    std::vector<duchamp::Section> readSectionInfo(std::string filename);

  }
}

#endif
