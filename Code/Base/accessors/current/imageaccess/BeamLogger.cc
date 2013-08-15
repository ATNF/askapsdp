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
#include <imageaccess/BeamLogger.h>
#include <imageaccess/CasaImageAccess.h>

#include <casa/Arrays/Vector.h>
#include <casa/Quanta/Quantum.h>
#include <Common/ParameterSet.h>
#include <string>
#include <vector>

#include <askap/AskapLogging.h>
ASKAP_LOGGER(logger, ".beamLogger");

namespace askap {
    
    namespace accessors {
	
	BeamLogger::BeamLogger():
	    itsFilename("")
	{
	}

	BeamLogger::BeamLogger(const LOFAR::ParameterSet &parset):
	    itsFilename(parset.getString("beamLog",""))
	{
	}

	BeamLogger::BeamLogger(const std::string &filename):
	    itsFilename(filename)
	{
	}

	BeamLogger::BeamLogger(const BeamLogger& other)
	{
	    this->operator=(other);
	}

	BeamLogger& BeamLogger::operator= (const BeamLogger& other)
	{
	    if(this == &other) return *this;
	    this->itsFilename = other.itsFilename;
	    this->itsImageList = other.itsImageList;
	    this->itsBeamList = other.itsBeamList;
	    return *this;
	}
	

	/// @details The beam information is extracted from each
	/// channel image and stored in a vector.
	void BeamLogger::extractBeams(std::vector<std::string> imageList)
	{
	    this->itsImageList = imageList;
	    this->itsBeamList.clear();
	    std::vector<std::string>::iterator image=this->itsImageList.begin();
	    for(;image!=this->itsImageList.end();image++){
		CasaImageAccess ia;
		this->itsBeamList.push_back(ia.beamInfo(*image));
	    }
	}


	/// @details The beam information for each channel image is
	/// written to the beam log. The log is in ASCII format, with
	/// each line having columns: number | image name | major axis
	/// [arcsec] | minor axis [arcsec] | position angle
	/// [deg]. Each column is separated by a single space. The
	/// first line is a comment line (starting with a '#') that
	/// indicates what each column contains.
	void BeamLogger::write()
	{
	    if(this->itsFilename != "") {
		
		std::ofstream fout(this->itsFilename.c_str());
		fout << "#Channel Image_name BMAJ[arcsec] BMIN[arcsec] BPA[deg]\n";
		for(size_t i=0;i<this->itsBeamList.size();i++){
		    fout << i << " " << this->itsImageList[i] << " " 
			 << this->itsBeamList[i][0].getValue("arcsec") << " " 
			 << this->itsBeamList[i][1].getValue("arcsec") << " " 
			 << this->itsBeamList[i][2].getValue("deg") <<"\n";
		}
		    
	    }
	    else 
		ASKAPLOG_ERROR_STR(logger, "BeamLogger cannot write the log, as no filename has been specified");

	}

	/// @details The beam log file is opened and each channel
	/// image's beam information is read and stored in the vector
	/// of beam values. The list of channel image names is also
	/// filled. If the beam log can not be opened, both vectors
	/// are cleared and an error message is written to the log.
	void BeamLogger::read()
	{
	    this->itsImageList.clear();
	    this->itsBeamList.clear();

	    if(this->itsFilename != ""){

		std::ifstream fin(this->itsFilename.c_str());
		if(fin.is_open()){

		    int chan;
		    double bmaj,bmin,bpa;
		    std::string line,name;
		    while(getline(fin,line),
			  !fin.eof()){
			std::stringstream ss(line);
			ss >> chan >> name >> bmaj >> bmin >> bpa;
			this->itsImageList.push_back(name);
			casa::Vector<casa::Quantum<double> > currentbeam(3);
			currentbeam[0]=casa::Quantum<double>(bmaj,"arcsec");
			currentbeam[1]=casa::Quantum<double>(bmin,"arcsec");
			currentbeam[2]=casa::Quantum<double>(bpa,"deg");
			this->itsBeamList.push_back(currentbeam);
		    }

		}
		else{
		    ASKAPLOG_ERROR_STR(logger, "Beam log file " << this->itsFilename << " could not be opened.");
		}

	    }

	}

    }

}

