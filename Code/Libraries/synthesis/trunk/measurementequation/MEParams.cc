#include <measurementequation/MEParams.h>
#include <iostream>

namespace conrad {
namespace synthesis
{
	
	/// Default constructor
	MEParams::MEParams() {
	}
	
	/// Copy constructor
	MEParams::MEParams(const MEParams& other) {
		operator=(other);
	}
	
	/// Assignment operator
	MEParams& MEParams::operator=(const MEParams& other) {
		if(this!=&other) {
			itsImage=other.itsImage;
			itsRegular=other.itsRegular;
		}
		return *this;
	}

	void MEParams::add(const string& name) {
		itsRegular.add(name, 0.0);
	};
	void MEParams::add(const string& name, const double& value) {
		itsRegular.add(name, value);
	};
	void MEParams::add(const string& name, const MEImage& value) {
		itsImage.add(name, value);
	};
	void MEParams::update(const string& name, const double& value) {
		itsRegular.update(name, value);
	};
	void MEParams::update(const string& name, const MEImage& value) {
		itsImage.update(name, value);
	};
	/// Return the regular parameters
	const MERegularParams& MEParams::regular() const {return itsRegular;};
	MERegularParams& MEParams::regular()  {return itsRegular;};

    /// Return the image parameters
	const MEImageParams& MEParams::image() const {return itsImage;};
	MEImageParams& MEParams::image() {return itsImage;};
	
	const uint MEParams::size() const {return itsRegular.size()+itsImage.size();};
	
	void MEParams::reset() {regular().reset();image().reset();};

	/// Is this set of parameters congruent with another?	
	bool MEParams::isCongruent(const MEParams& other) const {
		return itsRegular.isCongruent(other.regular()) && 
			itsImage.isCongruent(other.image());
	};

}
}
