/// @file
///
/// MEParams: represent a set of parameters for an imaging equation.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef MEPARAMS_H_
#define MEPARAMS_H_

#include <measurementequation/MEParamsRep.h>
#include <measurementequation/MEImage.h>

namespace conrad {
namespace synthesis
{

typedef MEParamsRep<MEImage> MEImageParams; 
typedef MEParamsRep<double> MERegularParams;

class MEParams {
public:

	/// Default constructor
	MEParams();
	
	/// Copy constructor
	MEParams(const MEParams& other);
	
	/// Assignment operator
	MEParams& operator=(const MEParams& other);

	/// Add a (regular) parameter, default to zero
	/// @param name Name of the parameter
	void add(const string& name);

	/// Add a (regular) parameter with a specific value
	/// @param name Name of the parameter
	/// @param value Value of the parameter
	void add(const string& name, const double& value);

	/// Add an image parameter
	/// @param name Name of the parameter
	/// @param value MEImage
	void add(const string& name, const MEImage& value);

	/// Update a (regular) parameter with a specific value
	/// @param name Name of the parameter
	/// @param value Value of the parameter
	void update(const string& name, const double& value);

	/// Update an image parameter
	/// @param name Name of the parameter
	/// @param value MEImage
	void update(const string& name, const MEImage& value);

	/// Return the regular parameters
	const MERegularParams& regular() const;
	MERegularParams& regular();

    /// Return the image parameters
	const MEImageParams& image() const;
	MEImageParams& image();
	
	/// Return the total number of parameters
	const uint size() const;
	
	/// Reset to empty
	void reset();

	/// Is this set of parameters congruent with another?	
	bool isCongruent(const MEParams& other) const;
	
private:
	MERegularParams itsRegular;
	MEImageParams itsImage;
};

}
}

#endif /*MEPARAMS_H_*/
