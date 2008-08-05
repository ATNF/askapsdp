/// @file
///
/// Provides generic methods for pattern matching 
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
	/// @brief Default constructor
	Point(){itsFlux=0.; itsID="";};
	/// @brief Constructor from position
	Point(double x, double y){itsX=x; itsY=y; itsFlux=0.; itsID="";};
	/// @brief Constructor from position & flux
	Point(double x, double y, double f){itsX=x; itsY=y; itsFlux=f;};
	/// @brief Constructor from position, flux, ID
	Point(double x, double y, double f, std::string id){itsX=x; itsY=y; itsFlux=f; itsID=id;};
	/// @brief Constructor from position, flux, ID and elliptical shape
	Point(double x, double y, double f, std::string id, double maj, double min, double pa){itsX=x; itsY=y; itsFlux=f; itsID=id; itsMajAxis=maj; itsMinAxis=min; itsPA=pa;};
	/// @brief Copy Constructor
	Point(const Point& p){operator=(p);};
	/// @brief Copy function
	Point& operator= (const Point& p);
	/// @brief Destructor
	~Point(){};

	/// @brief Set the x coordinate
	void setX(double x){itsX=x;};
	/// @brief Return the x coordinate
	double x(){return itsX;};
	/// @brief Set the y coordinate
	void setY(double y){itsY=y;};
	/// @brief Return the y coordinate
	double y(){return itsY;};
	/// @brief Set the flux
	void setFlux(double f){itsFlux=f;};
	/// @brief Return the flux
	double flux(){return itsFlux;};
	/// @brief Set the major axis
	void setMajorAxis(double a){itsMajAxis=a;};
	/// @brief Return the major axis
	double majorAxis(){return itsMajAxis;};
	/// @brief Set the minor axis
	void setMinorAxis(double a){itsMinAxis=a;};
	/// @brief Return the minor axis
	double minorAxis(){return itsMinAxis;};
	/// @brief Set the position angle
	void setPA(double a){itsPA=a;};
	/// @brief Return the position angle
	double PA(){return itsPA;};
	/// @brief Set the ID string
	void setID(std::string id){itsID=id;};
	/// @brief Return the ID string
	std::string ID(){return itsID;};

	/// @brief Less-than function, operating on the flux
	friend bool operator<(Point lhs, Point rhs){return lhs.flux()<rhs.flux();};

	/// @brief Return the separation from another Point.
	double sep(Point pt){return hypot(itsX-pt.x(),itsY-pt.y());};

      protected:
	///@brief The X coordinate
	double itsX;
	///@brief The Y coordinate
	double itsY;
	///@brief The peak flux of the point
	double itsFlux;
	///@brief The identification string
	std::string itsID;
	///@brief The major axis of a Gaussian fit
	double itsMajAxis;
	///@brief The minor axis of a Gaussian fit
	double itsMinAxis;
	///@brief The position angle of a Gaussian fit
	double itsPA;
      };


      /// @brief A class to hold info on a triangle side (a straight line)
      /// @details This class holds the necessary information on a
      ///  line connecting two points, providing functions to access its length,
      ///  dx and dy.
      class Side
      {
      public:
	/// @brief Default constructor
	Side(){};
	/// @brief Constructor using rise over run
	Side(double run, double rise){dx=run; dy=rise;};
	/// @brief Destructor
	~Side(){};

	/// @brief Definition function, using slope (defined by rise and run)
	void define(double run, double rise){dx=run; dy=rise;};
	/// @brief Definition function, using two points
	void define(Point a, Point b){dx=a.x()-b.x(); dy=a.y()-b.y();};

	/// @brief Return the rise (delta-y)
	double rise(){return dy;};
	/// @brief Return the run (delta-x)
	double run(){return dx;};
	/// @brief Return the length of the side
	double length(){return hypot(dx,dy);};
	/// @brief Less-than function, working on length of the sides.
	friend bool operator<(Side lhs, Side rhs){return lhs.length()<rhs.length();};

      protected:
	/// @brief Length in x-direction
	double dx;
	/// @brief Length in y-direction
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
	/// @brief Default constructor
	Triangle();
	/// @brief Constructor from three Points
	Triangle(Point pt1, Point pt2, Point pt3);
	/// @brief Constructor from three positions
	Triangle(double x1, double y1, double x2, double y2, double x3, double y3);
	/// @brief Copy constructor
	Triangle(const Triangle& t){operator=(t);};
	/// @brief Copy function
	Triangle& operator= (const Triangle& t);
	/// @brief Destructor
	~Triangle(){};
	/// @brief Definition function using three Points
	void define(Point pt1, Point pt2, Point pt3);

	/// @brief Calculate tolerances for triangle parameters.
	void defineTolerances(double epsilon=posTolerance);

	/// @brief Does this triangle match another?
	bool isMatch(Triangle &comp, double epsilon=posTolerance);

	/// @brief Return the ratio of longest to shortest sides
	double ratio(){return itsRatio;};
	/// @brief Return the tolerance for the ratio value
	double ratioTol(){return itsRatioTolerance;};
	/// @brief Return the angle
	double angle(){return itsAngle;};
	/// @brief Return the tolerance for the angle value
	double angleTol(){return itsAngleTolerance;};
	/// @brief Is the sense of the triangle clockwise?
	double isClockwise(){return itIsClockwise;};
	/// @brief Return the log of the perimeter
	double perimeter(){return itsLogPerimeter;};

	/// @brief Return the first point
	Point one(){return itsPts[0];};
	/// @brief Return the second point
	Point two(){return itsPts[1];};
	/// @brief Return the third point
	Point three(){return itsPts[2];};
	/// @brief Return the list of Points
	std::vector<Point> getPtList(){return itsPts;};

	/// @brief Less-than function, working on ratio values.
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

