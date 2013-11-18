/// @file
///
/// Class to calculate pixel fluxes for a uniform-surface-brightness elliptical disc
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#ifndef ASKAP_ANALYSIS_DISC_H_
#define ASKAP_ANALYSIS_DISC_H_
#include <modelcomponents/DiscEllipse.h>
#include <modelcomponents/DiscPixel.h>
#include <modelcomponents/Spectrum.h>
#include <Common/ParameterSet.h>

namespace askap { 
    
    namespace analysisutilities {


	class Disc
	{
	public:
	    Disc();
	    Disc(const LOFAR::ParameterSet &parset);
	    Disc(const Disc& other);
	    Disc& operator= (const Disc& other);
	    virtual ~Disc(){};

	    void setup(double x0, double y0, double maj, double min, double pa);
	    /* void setup(Spectrum *spec, double x0, double y0); */
	    /* void populate(float *arr); */

	    int xmin(){return itsEllipse.xmin();};
	    int xmax(){return itsEllipse.xmax();};
	    int ymin(){return itsEllipse.ymin();};
	    int ymax(){return itsEllipse.ymax();};

	    double flux(int x, int y);

	    Ellipse &ellipse(){return itsEllipse;};
	    friend std::ostream& operator<<(std::ostream &theStream, Disc &disc);

	protected:
	    DiscEllipse itsEllipse;
	    std::vector<DiscPixel> itsPixelSet;

	    double itsResolutionLimit;
	    double itsTresolution;   // number of points to divide the t-range by
	    unsigned int itsDecimationFactor;

	};


    }

}
#endif
