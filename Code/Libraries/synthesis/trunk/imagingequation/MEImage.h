/// @file
///
/// MEImage: Represent an image for imaging equation purposes.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef IEQIMAGE_H_
#define IEQIMAGE_H_

#include <iostream.h>

#include <images/Images/TempImage.h>

namespace conrad
{

typedef casa::Float MEImagePixelType ;

class MEImage : public casa::TempImage<MEImagePixelType>
{
public:
	MEImage();
	virtual ~MEImage();
};

}

#endif /*IEQIMAGE_H_*/
