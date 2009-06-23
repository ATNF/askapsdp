/// @file makecube.cc
///
/// @copyright (c) 2009 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// Include package level header file
#include <askap_imager.h>

// System includes
#include <cstdlib>

// caacore includes
#include <casa/Arrays/Array.h>
#include <casa/Quanta/Unit.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/PagedImage.h>

std::string getName(const std::string& base, int chan)
{
        std::stringstream name;
        name << base << chan;

        return name.str();
}

casa::CoordinateSystem getCoordinameSystem(const std::string& name)
{
    casa::PagedImage<float> img(name);
    return img.coordinates();
}

casa::Unit getUnits(const std::string& name)
{
    casa::PagedImage<float> img(name);
    return img.units();
}


casa::IPosition getShape(const std::string& name)
{
    casa::PagedImage<float> img(name);
    return img.shape();
}

int main(int argc, char *argv[])
{
    if (argc != 4) {
        std::cerr << "usage: " << argv[0]
            << " <image base name> <# channels> <output cube name>"
            << std::endl;
        return 1;
    }

    // Parameters
    const std::string imageBase(argv[1]);
    const int nChan = atoi(argv[2]);
    const std::string name(argv[3]);

    // Name of first image. This image is used to get the coordinate system
    // and the dimensions of the input images. We assume that all these images
    // have the same coordinate systems and dimensions, although that may not
    // be correct
    std::string refImageName = getName(imageBase, 1);
    casa::IPosition refShape = getShape(refImageName);
    if (refShape(0) != refShape(1)) {
        std::cout << "Error: Input images must be square in i & j dimensions"
            << std::endl;
        return 1;
    }
    const int xyDims = refShape(0);
    const casa::CoordinateSystem csys = getCoordinameSystem(refImageName);
    const casa::Unit units = getUnits(refImageName);

    // Create new image cube
    const casa::IPosition cubeShape(4, xyDims, xyDims, 1, nChan);

    double size = static_cast<double>(xyDims) * xyDims * nChan * sizeof(float);
    size = size / 1024.0 / 1024.0 / 1024.0;
    std::cout << "Creating image cube of size ~" << size
        << "GB. This may take a few minutes." << std::endl;

    casa::PagedImage<float> cube(casa::TiledShape(cubeShape), csys, name);
    cube.setUnits(units);

    // Open source images and write the slices into the cube
    for (int i = 1 ; i <= nChan; ++i) {
        std::string name = getName(imageBase, i);
        std::cout << "Adding slice from image " << name << std::endl;
        casa::PagedImage<float> img(name);

        // Ensure shape is the same
        if (img.shape() != refShape) {
            std::cout << "Error: Input images must all have the same shape"
                << std::endl;
            return 1;
        }

        // Ensure coordinate system is the same
        /*
        if (img.coordinates() != csys) {
            std::cout << "Error: Input images must all have the same coordinate system"
                << std::endl;
            return 1;
        }
        */

        // Ensure units are the same
        if (img.units() != units) {
            std::cout << "Error: Input images must all have the same units"
                << std::endl;
            return 1;
        }

        casa::Array<float> arr = img.get();
        casa::IPosition where(4, 0, 0, 0, i-1);
        cube.putSlice(arr, where);
    }

    return 0;
}
