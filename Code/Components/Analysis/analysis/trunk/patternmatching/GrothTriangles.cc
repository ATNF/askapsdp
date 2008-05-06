/// @file
///
/// Provides generic methods for pattern matching 
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///

#include <patternmatching/GrothTriangles.h>

#include <duchamp/PixelMap/Voxel.hh>

#include <math.h>
#include <vector>
#include <string>

namespace askap
{
  namespace analysis
  {

    namespace matching
    {

      Triangle::Triangle(PixelInfo::Voxel a, PixelInfo::Voxel b, PixelInfo::Voxel c)
      {
	this->define(a.getX(),a.getY(),b.getX(),b.getY(),c.getX(),c.getY());
      }

      Triangle::Triangle(float x1, float y1, float x2, float y2, float x3, float y3)
      {
	this->define(x1,y1,x2,y2,x3,y3);
      }

      void Triangle::define(float x1, float y1, float x2, float y2, float x3, float y3)
      {

	std::vector<Side> sides(3);
	sides[0].define(x1-x2,y1-y2);
	sides[1].define(x3-x2,y3-y2);
	sides[2].define(x1-x3,y1-y3);
	std::sort(sides.begin(), sides.end());
	// the sides are now ordered, so that the first is the shortest

	// use terminology from Groth 1986, where r2=shortest side, r3=longest side
	double r2= sides.begin()->length(), r3= sides.rbegin()->length();
	double dx2=sides.begin()->run(),    dx3=sides.rbegin()->run();
	double dy2=sides.begin()->rise(),   dy3=sides.rbegin()->rise();
// 	std::cerr <<"\n"<< r2<<" "<<r3<<" "<<dx2<<" "<<dy2<<" "<<dx3<<" "<<dy3<<"\n";

	this->itsRatio = r3/r2;

	this->itsAngle = (dx3*dx2 + dy3*dy2) / (r3*r2);
	double sinthetaSqd = 1. - this->itsAngle*this->itsAngle;

	double factor = 1./(r3*r3) - this->itsAngle/(r3*r2) + 1./(r2*r2);

	this->itsRatioTolerance = 2. * this->itsRatio * this->itsRatio * posTolerance * posTolerance * factor;

	this->itsAngleTolerance = 2. * sinthetaSqd * posTolerance * posTolerance * factor +
	  3. * this->itsAngle * this->itsAngle * pow(posTolerance,4) * factor * factor;

// 	std::cerr << factor << "\n";
// 	std::cerr << "! "<<itsRatio<<" "<<itsRatioTolerance<<" "<<itsAngle<<" "<<itsAngleTolerance<<"\n";


	double sum=0.;
	for(int i=0;i<3;i++) sum += sides[i].length();
	this->itsLogPerimeter = log10(sum);

	double tantheta = (dy2*dx3 - dy3*dx2) / (dx2*dx3 + dy2*dy3);
// 	std::cerr << "tantheta = " << tantheta << "\n";
	this->itIsClockwise = (tantheta > 0.);

      }


      bool Triangle::isMatch(Triangle &comp)
      {
	double ratioSep = this->itsRatio - comp.ratio();
	ratioSep *= ratioSep;
	double ratioTol = this->itsRatioTolerance + comp.ratioTol();
	double angleSep = this->itsAngle - comp.angle();
	angleSep *= angleSep;
	double angleTol = this->itsAngleTolerance + comp.angleTol();

//  	std::cerr << "match: "<<ratioSep<<" "<<ratioTol<<" "<<angleSep<<" "<<angleTol<<"\n";

	return ((ratioSep < ratioTol) && (angleSep < angleTol));
      }

    }
  }
}
