/// @file
///
/// Class to log the restoring beams of individual channels of a spectral cube
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
#ifndef BEAM_LOGGER_H
#define BEAM_LOGGER_H

#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Quantum.h>
#include <Common/ParameterSet.h>
#include <string>
#include <vector>

namespace askap {
    
    namespace accessors {
	
	/// @brief Class to handle writing & reading of channel-level
	/// beam information for a spectral cube.  
	/// @details This class wraps up the functionality required to
	/// create and access the beam log files. These files are
	/// created by the makecube application to record the
	/// restoring beam of the individual channel images that are
	/// combined to form the spectral cube. The class also
	/// provides the ability to straightforwardly read the beam
	/// log to extract the channel-level beam information.

	class BeamLogger
	{
	public:
	    BeamLogger();
	    BeamLogger(const LOFAR::ParameterSet &parset);
	    BeamLogger(const std::string &filename);
	    BeamLogger(const BeamLogger& other);
	    BeamLogger& operator= (const BeamLogger& other);
	    virtual ~BeamLogger(){};

	    /// Set the name of the beam log file
	    void setFilename(std::string filename){itsFilename=filename;};

	    /// @brief Extract the beam information for each channel image
	    void extractBeams(std::vector<std::string> imageList);

	    /// @brief Write the beam information to the beam log
	    void write();

	    /// @brief Read the beam information from a beam log
	    void read();

	    /// Return the beam information
	    std::vector< casa::Vector<casa::Quantum<double> > > beamlist(){return itsBeamList;};

	    /// Return the list of channel images
	    std::vector<std::string> imageList(){return itsImageList;};

	protected:
	    std::string itsFilename;
	    std::vector<std::string> itsImageList;
	    std::vector< casa::Vector<casa::Quantum<double> > > itsBeamList;
	    
	};

    }

}

#endif   // #ifndef BEAM_LOGGER_H
