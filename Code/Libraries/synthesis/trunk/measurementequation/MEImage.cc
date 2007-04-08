#include <measurementequation/MEImage.h>

namespace conrad
{
namespace synthesis
{
	
MEImage::MEImage() : itsName("")
{
}

MEImage::MEImage(const string& name) : itsName(name)
{
}

MEImage::MEImage(const MEImage& other) 
{
	operator=(other);
}

MEImage& MEImage::operator=(const MEImage& other) 
{
	if(this!=&other) {
		itsName=other.itsName;
	}
	return *this;
}

MEImage::~MEImage()
{
}

}
}