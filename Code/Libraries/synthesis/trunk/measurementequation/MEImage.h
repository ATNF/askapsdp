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

namespace conrad {
namespace synthesis {

typedef casa::Float MEImagePixelType ;

class MEImage {

public:
	MEImage();
	MEImage(const string& name);
	MEImage(const MEImage& other);
	MEImage& operator=(const MEImage& other);
	virtual ~MEImage();
	bool operator==(const MEImage& other);
	
private:
	string itsName;
//	casa::TempImage<MEImagePixelType> itsCache;
};

}
}

#endif /*MEIMAGE_H_*/
