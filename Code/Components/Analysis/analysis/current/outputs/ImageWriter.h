/// @file
///
/// Utility class to easily write out a CASA image, with optional piece-wise writing
///
/// @copyright (c) 2011 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///

#ifndef ASKAP_ANALYSIS_IMAGE_WRITER_H_
#define ASKAP_ANALYSIS_IMAGE_WRITER_H_

#include <string>
#include <duchamp/Cubes/cubes.hh>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/IPosition.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/ImageInfo.h>
#include <casa/aipstype.h>

namespace askap {

    namespace analysis { 

	class ImageWriter
	{
	public:
	    ImageWriter(){};
	    ImageWriter(const ImageWriter& other);
	    ImageWriter& operator= (const ImageWriter& other);
	    virtual ~ImageWriter(){};

	    
	    void copyMetadata(duchamp::Cube *cube);

	    std::string &imagename(){return itsImageName;};
	    casa::Unit &bunit(){return itsBunit;};
//	    std::string bunit(){return itsBunit.getName();};
	    casa::CoordinateSystem &coordsys(){return itsCoordSys;};
	    casa::IPosition &shape(){return itsShape;};
	    
	    void setTileshapeFromShape(casa::IPosition &shape);

	    void create(std::string filename);
	    
	    void write(float *data, casa::IPosition &shape);
	    void write(float *data, casa::IPosition &shape, casa::IPosition &loc);
	    void write(casa::Array<casa::Float> &data);
	    void write(casa::Array<casa::Float> &data, casa::IPosition &loc);

	protected:
	    std::string itsImageName;
	    casa::Unit itsBunit;
	    casa::IPosition itsShape;
	    casa::IPosition itsTileshape;
	    casa::CoordinateSystem itsCoordSys;
	    casa::ImageInfo itsImageInfo;
	};


    }

}



#endif
