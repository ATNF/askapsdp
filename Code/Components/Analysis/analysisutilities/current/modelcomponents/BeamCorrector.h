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

#include <modelcomponents/ContinuumSelavy.h>

#include <duchamp/FitsIO/Beam.hh>
#include <Common/ParameterSet.h>

#include <string>
#include <vector>


namespace askap {

    namespace analysisutilities {

      /// @brief A class to manage access to the beam information of
      /// an image when dealing with a catalogue made from fits to that
      /// image.

      /// @details This class allows one to straightforwardly access
      /// the beam information from an image that was used to create a 
      /// catalogue. The simplest interface to creating a BeamCorrector object is
      /// to use the parset, where it looks for the BeamCorrector.Image parameter, being
      /// the name of the file in question. It will then read the beam and
      /// spatial pixel scale from that image. It can then be used to correct
      /// the flux of sources created from the catalogue, converting their flux
      /// from Jy to Jy/beam so that they can be placed onto an image.

      class BeamCorrector {
      public:
	/// @brief Default constructor
	BeamCorrector();
	/// @brief Parset-based constructor
	BeamCorrector(const LOFAR::ParameterSet& parset);
	/// @brief Copy constructor
	BeamCorrector(const BeamCorrector& other);
	/// @brief Assignment constructor
	BeamCorrector& operator= (const BeamCorrector& other);
	/// @brief Default destructor
	virtual ~BeamCorrector(){};

	/// @brief Read the beam & pixel scale from the image
	void findBeam();
	/// @brief Convert the flux of a source
	void convertSource(Spectrum *src);
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
