/// @file
///
/// Provides generic methods for pattern matching 
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef ASKAP_ANALYSIS_GROTHTRIANGLES_H_
#define ASKAP_ANALYSIS_GROTHTRIANGLES_H_

#include <duchamp/PixelMap/Voxel.hh>

#include <vector>
#include <string>
#include <math.h>

namespace askap
{
  namespace analysis
  {

    namespace matching
    {

      class Side
      {
      public:
	Side(){};
	Side(float run, float rise){dx=run; dy=rise;};
	void define(float run, float rise){dx=run; dy=rise;};
	float rise(){return dy;};
	float run(){return dx;};
	float length(){return hypot(dx,dy);};
	friend bool operator<(Side lhs, Side rhs){return lhs.length()<rhs.length();};

      protected:
	float dx;
	float dy;
      };


      const double posTolerance = 0.001;
      const double elimThreshold = 0.003;


      /// @brief Support class for matching patterns of sources
      ///
      /// @details This class holds all the information to specify a
      /// triangle of points from a list of sources that can be matched to
      /// another list. The formulation follows Groth 1986 (AJ 91, 1244-1248).

      class Triangle
      {
      public:
	Triangle(PixelInfo::Voxel a, PixelInfo::Voxel b, PixelInfo::Voxel c);
	Triangle(float x1, float y1, float x2, float y2, float x3, float y3);
	void define(float x1, float y1, float x2, float y2, float x3, float y3);

	bool isMatch(Triangle &comp);

	double ratio(){return itsRatio;};
	double ratioTol(){return itsRatioTolerance;};
	double angle(){return itsAngle;};
	double angleTol(){return itsAngleTolerance;};
	double isClockwise(){return itIsClockwise;};
	double perimeter(){return itsLogPerimeter;};

	friend operator< (Triangle lhs, Triangle rhs){return lhs.ratio()<rhs.ratio();};

      protected:

	double itsLogPerimeter;
	bool   itIsClockwise;
	double itsRatio;
	double itsRatioTolerance;
	double itsAngle;
	double itsAngleTolerance;
      
      };

    }
  }
}

#endif

