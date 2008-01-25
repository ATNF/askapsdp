/// @file
///
/// @brief General utility functions to support the analysis software
/// @details
/// These functions are unattached to any classes, but provide simple
/// support for the rest of the analysis package.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Matthew Whiting <matthew.whiting@csiro.au>
/// 

#include <analysisutilities/AnalysisUtilities.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <duchamp/Utils/Statistics.hh>
#include <duchamp/Utils/Section.hh>

namespace conrad
{
  namespace analysis
  {

    
    double findSpread(bool robust, double middle, int size, float *array)
    {
      /// Finds the "spread" (ie. the rms or standard deviation) of an
      /// array of values using a given mean value. The option exists
      /// to use the standard deviation, or, by setting robust=true,
      /// the median absolute deviation from the median. In the latter
      /// case, the middle value given is assumed to be the median,
      /// and the returned value is the median absolute difference of
      /// the data values from the median.

      double spread=0.;
      if(robust){
	float *arrayCopy = new float[size];
	for(int i=0;i<size;i++) arrayCopy[i] = fabs(array[i]-middle);
	std::sort(arrayCopy,arrayCopy+size);
	if((size%2)==0) spread = (arrayCopy[size/2-1]+arrayCopy[size/2])/2;
	else spread = arrayCopy[size/2];
	delete [] arrayCopy;
	spread = Statistics::madfmToSigma(spread);
      }
      else{
	for(int i=0;i<size;i++) spread += (array[i]-middle)*(array[i]-middle);
	spread = sqrt(spread / double(size-1));
      }
      return spread;
    }


    std::vector<duchamp::Section> readSectionInfo(std::string filename)
    {
      std::ifstream fin(filename.c_str());
      int numAxes;
      fin >> numAxes;
      std::vector<long> dimAxes(numAxes);
      for(int i=0;i<numAxes;i++) fin>>dimAxes[i];
      std::vector<duchamp::Section> sectionlist;
      while(!fin.eof()){
	std::string image,sectionString;
	fin >> image >> sectionString;
	if(!fin.eof()){
	  duchamp::Section section(sectionString);
	  section.parse(dimAxes);
	  sectionlist.push_back(section);
	}
      }
      fin.close();
      return sectionlist;

    }


  }
}
