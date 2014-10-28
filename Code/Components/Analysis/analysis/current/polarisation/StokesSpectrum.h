/// @file
///
/// Holds spectral information for a given source for a given Stokes parameter
///
/// @copyright (c) 2014 CSIRO
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
#ifndef ASKAP_ANALYSIS_STOKES_SPECTRUM_H_
#define ASKAP_ANALYSIS_STOKES_SPECTRUM_H_

namespace askap {

    namespace analysis { 

	class StokesSpectrum
	{
	public:
	    StokesSpectrum(const LOFAR::ParameterSet &parset);
	    StokesSpectrum(const StokesSpectrum& other);
	    StokesSpectrum& operator= (const StokesSpectrum& other);
	    virtual ~StokesSpectrum();

	    void initialise(RadioSource *src);

	    casa::Array<float> spectrum(){return itsSpectrum;};
	    casa::Array<float> noiseSpectrum(){return itsNoiseSpectrum;};
	    float median(){return itsMedianValue;};
	    float medianNoise(){return itsMedianNoise;};


	private:

	    RadioSource *itsSrc;
	    std::string itsCubeName;
	    unsigned int itsBoxWidth;
	    casa::Array<float> itsSpectrum;
	    float itsMedianValue;
	    casa::Array<float> itsNoiseSpectrum;
	    float itsMedianNoise;

	};

    }

}

#endif
