/// @file
///
/// MEDesignMatrix: represent a set of parameters for an imaging equation.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEDESIGNMATRIX_H_
#define MEDESIGNMATRIX_H_

#include <measurementequation/MEDesignMatrixRep.h>
#include <measurementequation/MEImage.h>
#include <measurementequation/MEParams.h>

namespace conrad {
namespace synthesis
{

typedef MEDesignMatrixRep<MEImage> MEImageDesignMatrix; 
typedef MEDesignMatrixRep<double> MERegularDesignMatrix;

class MEDesignMatrix {
public:

	/// Default constructor
	MEDesignMatrix();

	/// Constructor from parameters
	MEDesignMatrix(const MEParams& ip);
	
	/// Copy constructor
	MEDesignMatrix(const MEDesignMatrix& other);
	
	/// Assignment operator
	MEDesignMatrix& operator=(const MEDesignMatrix& other);

	/// Return the regular parameters
	const MERegularDesignMatrix& regular() const;
	MERegularDesignMatrix& regular();

    /// Return the image parameters
	const MEImageDesignMatrix& image() const;
	MEImageDesignMatrix& image();
	
	/// Reset to empty
	void reset();

private:
	MERegularDesignMatrix itsRegular;
	MEImageDesignMatrix itsImage;
};

}
}

#endif
