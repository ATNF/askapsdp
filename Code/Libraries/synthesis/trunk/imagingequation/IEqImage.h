/// @file
///
/// IEqImage: Represent an image for imaging equation purposes.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef IEQIMAGE_H_
#define IEQIMAGE_H_

#include <images/Images/TempImage.h>

namespace conrad
{

typedef casa::Float IEqImagePixelType ;

class IEqImage : public casa::TempImage<IEqImagePixelType>
{
public:
	IEqImage();
	virtual ~IEqImage();
};

}

#endif /*IEQIMAGE_H_*/
