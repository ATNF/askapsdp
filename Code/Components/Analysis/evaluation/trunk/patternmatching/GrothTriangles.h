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

  namespace evaluation
  {

    namespace matching
    {

      /// @brief A class to hold information about a 2D point
      /// @details This class holds positional information that will
      ///  be used by the pattern matching algorithms. It holds information about
      ///  a single point in the plane: its X and Y coordinates, its flux (a
      ///  measure of its brightness or importance), and an identification
      ///  string.
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
	friend bool operator<(Point lhs, Point rhs){return lhs.flux()<rhs.flux();};

      protected:
	///@brief The X coordinate
	double itsX;
	///@brief The Y coordinate
	double itsY;
	///@brief The flux of the point
	double itsFlux;
	///@brief The identification string
	std::string itsID;
      };


      /// @brief A class to hold info on a triangle side (a straight line)
      /// @details This class holds the necessary information on a
      ///  line connecting two points, providing functions to access its length,
      ///  dx and dy.
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


      /// @brief The default tolerance in the position for triangle matching
      const double posTolerance = 0.001;
      /// @brief The default elimination threshold for culling lists prior to triangle matching
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

	/// @brief The log of the perimeter of the triangle
	double itsLogPerimeter;
	/// @brief Whether the sides increase in size in a clockwise fashion
	bool   itIsClockwise;
	/// @brief The ratio between the largest and smallest sides
	double itsRatio;
	/// @brief The tolerance in the ratio value
	double itsRatioTolerance;
	/// @brief The angle between the largest and smallest sides (actually cos(angle))
	double itsAngle;
	/// @brief The tolerance in the angle value
	double itsAngleTolerance;

	/// @brief The list of points making up the triangle
	std::vector<Point> itsPts;
      
      };


      /// @brief Create a list of triangles from a list of points
      std::vector<Triangle> getTriList(std::vector<Point> pixlist);

      /// @brief Match two lists of triangles
      std::vector<std::pair<Triangle,Triangle> > 
	matchLists(std::vector<Triangle> list1, std::vector<Triangle> list2, double epsilon);

      /// @brief Eliminate likely false matches from a triangle list
      void trimTriList(std::vector<std::pair<Triangle,Triangle> > &trilist);

      /// @brief Make the final assignment of matching points
      std::vector<std::pair<Point,Point> > vote(std::vector<std::pair<Triangle,Triangle> > trilist);


    }
  }
}

#endif

