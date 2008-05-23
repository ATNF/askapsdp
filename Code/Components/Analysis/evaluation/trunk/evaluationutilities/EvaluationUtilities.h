/// @file
///
/// Provides base class for handling the matching of lists of points
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef ASKAP_EVALUATION_EVALUTIL_H_
#define ASKAP_EVALUATION_EVALUTIL_H_

#include <askap_evaluation.h>

#include <patternmatching/GrothTriangles.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

namespace askap
{

  namespace evaluation
  {

    std::vector<matching::Point> getSrcPixList(std::ifstream &fin, std::string raBaseStr, std::string decBaseStr);

    std::vector<matching::Point> getPixList(std::ifstream &fin, std::string raBaseStr, std::string decBaseStr);


    /// Converts a string in the format +12:23:34.45 to a decimal angle in degrees.
    double dmsToDec(std::string input);   
    
    /// Converts a decimal into a dd:mm:ss.ss format.
    std::string decToDMS(const double input, std::string type="DEC", 
			 int secondPrecision=2, std::string separator=":");
    
    /// Find the angular separation of two sky positions
    double angularSeparation(const std::string ra1, const std::string dec1, 
			     const std::string ra2, const std::string dec2);
    
    /// Find the angular separation of two sky positions.
    double angularSeparation(double ra1, double dec1, double ra2, double dec2);
    
    /// Convert equatorial coordinates to Galactic.
    void equatorialToGalactic(double ra, double dec, double &gl, double &gb);
    
 
  }

}

#endif
