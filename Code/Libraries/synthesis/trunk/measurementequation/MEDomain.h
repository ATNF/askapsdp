/// @file
///
/// MEDomain: Represent a domain for imaging equation purposes.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEDOMAIN_H_
#define MEDOMAIN_H_

#include <iostream>

namespace conrad
{
	
class MEDomain
{
public:
  // Create an  x,y default domain of -1:1,-1:1..
  MEDomain();

  // Create an x,y domain.
  MEDomain (double startX, double endX, double startY, double endY);
  
  ~MEDomain();

  // Get offset and scale value.
  double offsetX() const
    { return itsOffsetX; }
  double scaleX() const
    { return itsScaleX; }
  double offsetY() const
    { return itsOffsetY; }
  double scaleY() const
    { return itsScaleY; }

  // Transform a value to its normalized value.
  double normalizeX (double value) const
    { return (value - itsOffsetX) / itsScaleX; }
  double normalizeY (double value) const
    { return (value - itsOffsetY) / itsScaleY; }

  // Get the start, end, and step of the domain.
  double startX() const
    { return itsOffsetX - itsScaleX; }
  double endX() const
    { return itsOffsetX + itsScaleX; }
  double sizeX() const
    { return 2*itsScaleX; }
  double startY() const
    { return itsOffsetY - itsScaleY; }
  double endY() const
    { return itsOffsetY + itsScaleY; }
  double sizeY() const
    { return 2*itsScaleY; }

  bool operator== (const MEDomain& that) const
  { return itsOffsetX == that.itsOffsetX  &&  itsScaleX == that.itsScaleX
       &&  itsOffsetY == that.itsOffsetY  &&  itsScaleY == that.itsScaleY; }

private:
  double itsOffsetX;
  double itsScaleX;
  double itsOffsetY;
  double itsScaleY;
  
};
};
#endif /*MEDOMAIN_H_*/