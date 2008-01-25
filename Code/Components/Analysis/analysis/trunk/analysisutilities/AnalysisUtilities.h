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

#include <duchamp/Utils/Section.hh>

namespace conrad
{
  namespace analysis
  {

    // Find an rms for an array given a mean value
    double findSpread(bool robust, double middle, int size, float *array);

    // Read in image sections and return a duchamp::Section object;
    std::vector<duchamp::Section> readSectionInfo(std::string filename);

  }
}

#endif
