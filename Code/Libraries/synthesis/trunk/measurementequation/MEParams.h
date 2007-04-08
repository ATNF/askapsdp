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

	void add(const string& name) {itsRegular.add(name, 0.0);};
	void add(const string& name, const double value) {itsRegular.add(name, value);};
	void add(const string& name, const MEImage& value) {itsImage.add(name, value);};
	void addImage(const string& name, const MEImage& value) {itsImage.add(name, value);};
	
	/// Return the regular parameters
	const MERegularParams& regular() const {return itsRegular;};
	MERegularParams& regular()  {return itsRegular;};

    /// Return the image parameters
	const MEImageParams& image() const {return itsImage;};
	MEImageParams& image() {return itsImage;};
	
	const uint size() const {return itsRegular.size()+itsImage.size();};

	/// Is this set of parameters congruent with another?	
	bool isCongruent(const MEParams& other) const {
		return itsRegular.isCongruent(other.regular()) && itsImage.isCongruent(other.image());
	};
	
private:
	MERegularParams itsRegular;
	MEImageParams itsImage;
};

}
}

#endif /*MEPARAMS_H_*/
