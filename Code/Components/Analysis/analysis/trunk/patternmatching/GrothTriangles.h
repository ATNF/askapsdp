/// @file
///
/// Provides generic methods for pattern matching 
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef ASKAP_ANALYSIS_GROTHTRIANGLES_H_
#define ASKAP_ANALYSIS_GROTHTRIANGLES_H_

#include <vector>
#include <utility>
#include <string>
#include <math.h>

namespace askap
{
  namespace analysis
  {

    namespace matching
    {

      class Point
      {
      public:
	Point(){itsFlux=0.; itsID="";};
	Point(double x, double y){itsX=x; itsY=y; itsFlux=0.; itsID="";};
	Point(double x, double y, double f){itsX=x; itsY=y; itsFlux=f;};
	Point(double x, double y, double f, std::string id){itsX=x; itsY=y; itsFlux=f; itsID=id;};
	Point(const Point& p){operator=(p);};
	Point& operator= (const Point& p);
	~Point(){};
	void setX(double x){itsX=x;};
	double x(){return itsX;};
	void setY(double y){itsY=y;};
	double y(){return itsY;};
	void setFlux(double f){itsFlux=f;};
	double flux(){return itsFlux;};
	void setID(std::string id){itsID=id;};
	std::string ID(){return itsID;};

      protected:
	double itsX;
	double itsY;
	double itsFlux;
	std::string itsID;
      };

      class Side
      {
      public:
	Side(){};
	Side(double run, double rise){dx=run; dy=rise;};
	~Side(){};
	void define(double run, double rise){dx=run; dy=rise;};
	void define(Point a, Point b){dx=a.x()-b.x(); dy=a.y()-b.y();};
	double rise(){return dy;};
	double run(){return dx;};
	double length(){return hypot(dx,dy);};
	friend bool operator<(Side lhs, Side rhs){return lhs.length()<rhs.length();};

      protected:
	double dx;
	double dy;
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
	Triangle();
	Triangle(Point pt1, Point pt2, Point pt3);
	Triangle(double x1, double y1, double x2, double y2, double x3, double y3);
	Triangle(const Triangle& t){operator=(t);};
	Triangle& operator= (const Triangle& t);
	~Triangle(){};
	void define(Point pt1, Point pt2, Point pt3);

	void defineTolerances(double tolerance=posTolerance);

	bool isMatch(Triangle &comp, double epsilon=posTolerance);

	double ratio(){return itsRatio;};
	double ratioTol(){return itsRatioTolerance;};
	double angle(){return itsAngle;};
	double angleTol(){return itsAngleTolerance;};
	double isClockwise(){return itIsClockwise;};
	double perimeter(){return itsLogPerimeter;};

	Point one(){return itsPts[0];};
	Point two(){return itsPts[1];};
	Point three(){return itsPts[2];};
	std::vector<Point> getPtList(){return itsPts;};

	friend bool operator< (Triangle lhs, Triangle rhs){return lhs.ratio()<rhs.ratio();};

      protected:

	double itsLogPerimeter;
	bool   itIsClockwise;
	double itsRatio;
	double itsRatioTolerance;
	double itsAngle;
	double itsAngleTolerance;

	std::vector<Point> itsPts;
      
      };


      std::vector<Triangle> getTriList(std::vector<Point> pixlist);
      std::vector<std::pair<Triangle,Triangle> > matchLists(std::vector<Triangle> list1, std::vector<Triangle> list2, double epsilon);
      void trimTriList(std::vector<std::pair<Triangle,Triangle> > &trilist);
      std::vector<std::pair<Point,Point> > vote(std::vector<std::pair<Triangle,Triangle> > trilist);


    }
  }
}

#endif

