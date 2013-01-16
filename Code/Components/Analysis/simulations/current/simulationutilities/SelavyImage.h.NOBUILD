/// @file
///
/// Simple class to manage metadata from image that generated a Selavy catalogue
///
/// @copyright (c) 2010 CSIRO
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

#ifndef ASKAP_SIMS_SELAVY_IMAGE_H_
#define ASKAP_SIMS_SELAVY_IMAGE_H_

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <simulationutilities/ContinuumSelavy.h>

#include <duchamp/FitsIO/Beam.hh>
#include <Common/ParameterSet.h>

#include <string>
#include <vector>


namespace askap {

    namespace simulations {

      /// @brief A class to manage access to an image used to create a
      /// Selavy catalogue.

      /// @details This class allows one to straightforwardly access
      /// the beam information from an image that was used to create a Selavy
      /// catalogue. The simplest interface to creating a SelavyImage object is
      /// to use the parset, where it looks for the Selavyimage parameter, being
      /// the name of the file in question. It will then read the beam and
      /// spatial pixel scale from that image. It can then be used to correct
      /// the flux of sources created from the catalogue, converting their flux
      /// from Jy to Jy/beam so that they can be placed onto an image.

      class SelavyImage {
      public:
	/// @brief Default constructor
	SelavyImage();
	/// @brief Parset-based constructor
	SelavyImage(const LOFAR::ParameterSet& parset);
	/// @brief Copy constructor
	SelavyImage(const SelavyImage& other);
	/// @brief Assignment constructor
	SelavyImage& operator= (const SelavyImage& other);
	/// @brief Default destructor
	virtual ~SelavyImage(){};

	/// @brief Read the beam & pixel scale from the image
	void findBeam();
	/// @brief Convert the flux of a source
	void convertSource(ContinuumSelavy &src);
	/// @brief Return the beam information
	std::vector<float> beam();

      protected:
	/// @brief The image file 
	std::string itsFilename;
	/// @brief The beam information
	duchamp::Beam itsBeam;
	/// @brief The pixel scale in the image
	float itsPixelScale;
	/// @brief The units of the spatial axes
	casa::String itsDirUnits;

      };

    }

}



#endif
