/// @file
///
/// Provides generic methods for pattern matching 
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///

#include <patternmatching/GrothTriangles.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <string>

namespace askap
{
  namespace analysis
  {

    namespace matching
    {

      Point& Point::operator=(const Point& p)
      {
	if(this==&p) return *this;
	this->itsX = p.itsX;
	this->itsY = p.itsY;
	this->itsFlux = p.itsFlux;
	this->itsID = p.itsID;
	return *this;
      }

      Triangle::Triangle()
      {
	std::vector<Point> pts(3); this->itsPts=pts;
      }

      Triangle::Triangle(Point a, Point b, Point c)
      {
	std::vector<Point> pts(3); this->itsPts=pts;
	this->define(a,b,c);
      }

      Triangle::Triangle(double x1, double y1, double x2, double y2, double x3, double y3)
      {
	std::vector<Point> pts(3); this->itsPts=pts;
  	Point pt1(x1,y1);
	Point pt2(x2,y2);
	Point pt3(x3,y3);
	this->define(pt1,pt2,pt3);
      }

      Triangle& Triangle::operator= (const Triangle& t)
      {
	if(this==&t) return *this;
	this->itsLogPerimeter = t.itsLogPerimeter;
	this->itIsClockwise = t.itIsClockwise;
	this->itsRatio = t.itsRatio;
	this->itsRatioTolerance = t.itsRatioTolerance;
	this->itsAngle = t.itsAngle;
	this->itsAngleTolerance = t.itsAngleTolerance;
	this->itsPts = t.itsPts;
	return *this;
      }


      void Triangle::define(Point a, Point b, Point c)
      {
	std::vector<Point> ptslist(3);
  	ptslist[0]=a;
	ptslist[1]=b;
	ptslist[2]=c;
	std::vector<Side> sides(3);
	sides[0].define(a,b);
	sides[1].define(b,c);
	sides[2].define(c,a);
	std::vector<short> vote(3);
	for(int i=0;i<3;i++){
	  if( (min_element(sides.begin(), sides.end()) - sides.begin()) == i) vote[i]=1;
	  else if( (max_element(sides.begin(), sides.end()) - sides.begin()) == i) vote[i]=3;
	  else vote[i] = 2;
	}
	short matrix[3][3];
	for(int i=0;i<3;i++) matrix[i][i]=0;
	matrix[1][0] = matrix[0][1] = vote[0];
	matrix[2][1] = matrix[1][2] = vote[1];
	matrix[0][2] = matrix[2][0] = vote[2];

	for(int i=0;i<3;i++){
	  int sum=0;
	  for(int j=0;j<3;j++) sum += matrix[i][j];
	  switch(sum){
	  case 4: this->itsPts[0] = ptslist[i]; break;
	  case 3: this->itsPts[1] = ptslist[i]; break;
	  case 5: this->itsPts[2] = ptslist[i]; break;
	  }
	}

// 	std::cout << this->one().x() << " " << this->one().y() << "\n" 
// 		  << this->two().x() << " " << this->two().y() << "\n" 
// 		  << this->three().x() << " " << this->three().y() << "\n\n";

	std::sort(sides.begin(), sides.end());
	
	// the sides are now ordered, so that the first is the shortest

	// use terminology from Groth 1986, where r2=shortest side, r3=longest side
	double r2= sides.begin()->length(), r3= sides.rbegin()->length();
	double dx2=sides.begin()->run(),    dx3=sides.rbegin()->run();
	double dy2=sides.begin()->rise(),   dy3=sides.rbegin()->rise();
// 	std::cerr <<"\n"<< r2<<" "<<r3<<" "<<dx2<<" "<<dy2<<" "<<dx3<<" "<<dy3<<"\n";

	this->itsRatio = r3/r2;

	this->itsAngle = (dx3*dx2 + dy3*dy2) / (r3*r2);

// 	std::cerr << factor << "\n";
// 	std::cerr << "! "<<itsRatio<<" "<<itsRatioTolerance<<" "<<itsAngle<<" "<<itsAngleTolerance<<"\n";


	double sum=0.;
	for(int i=0;i<3;i++) sum += sides[i].length();
	this->itsLogPerimeter = log10(sum);

	double tantheta = (dy2*dx3 - dy3*dx2) / (dx2*dx3 + dy2*dy3);
// 	std::cerr << "tantheta = " << tantheta << "\n";
	this->itIsClockwise = (tantheta > 0.);

	defineTolerances();

      }

      void Triangle::defineTolerances(double epsilon)
      {

	Side side1_2(itsPts[0].x()-itsPts[1].x(),itsPts[0].y()-itsPts[1].y());
	Side side1_3(itsPts[0].x()-itsPts[2].x(),itsPts[0].y()-itsPts[2].y());
	double r2=side1_2.length(),r3=side1_3.length();

	double sinthetaSqd = 1. - this->itsAngle*this->itsAngle;

	double factor = 1./(r3*r3) - this->itsAngle/(r3*r2) + 1./(r2*r2);

	this->itsRatioTolerance = 2. * this->itsRatio * this->itsRatio * epsilon * epsilon * factor;

	this->itsAngleTolerance = 2. * sinthetaSqd * epsilon * epsilon * factor +
	  3. * this->itsAngle * this->itsAngle * pow(epsilon,4) * factor * factor;
	

      }

      bool Triangle::isMatch(Triangle &comp, double epsilon)
      {
	defineTolerances(epsilon);
	
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
