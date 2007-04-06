/// @file
///
/// MEImage: Represent an image for imaging equation purposes.
/// 
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEIMAGE_H_
#define MEIMAGE_H_

#include <images/Images/TempImage.h>

namespace conrad
{
namespace synthesis
{

typedef casa::Float MEImagePixelType ;

class MEImage : public casa::TempImage<MEImagePixelType>
{
public:
	MEImage();
	virtual ~MEImage();
};

}
}

#endif /*MEIMAGE_H_*/
