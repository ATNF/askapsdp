#include <measurementequation/MEDesignMatrix.h>
#include <iostream>

namespace conrad {
namespace synthesis
{
	
	/// Default constructor
	MEDesignMatrix::MEDesignMatrix() {
	}

	MEDesignMatrix::MEDesignMatrix(const MEParams& ip) :
		itsRegular(ip.regular()), itsImage(ip.image())
	{
	}
	
	/// Copy constructor
	MEDesignMatrix::MEDesignMatrix(const MEDesignMatrix& other) {
		operator=(other);
	}
	
	/// Assignment operator
	MEDesignMatrix& MEDesignMatrix::operator=(const MEDesignMatrix& other) {
		if(this!=&other) {
			itsImage=other.itsImage;
			itsRegular=other.itsRegular;
		}
		return *this;
	}

	/// Return the regular parameters
	const MERegularDesignMatrix& MEDesignMatrix::regular() const {return itsRegular;};
	MERegularDesignMatrix& MEDesignMatrix::regular()  {return itsRegular;};

    /// Return the image parameters
	const MEImageDesignMatrix& MEDesignMatrix::image() const {return itsImage;};
	MEImageDesignMatrix& MEDesignMatrix::image() {return itsImage;};
	
	void MEDesignMatrix::reset() {regular().reset();image().reset();};

}
}
