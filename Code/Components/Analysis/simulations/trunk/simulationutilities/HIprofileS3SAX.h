/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2008 CSIRO
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
#ifndef ASKAP_SIMS_HI_PROFILE_S3SAX_H_
#define ASKAP_SIMS_HI_PROFILE_S3SAX_H_

#include <simulationutilities/Spectrum.h>
#include <simulationutilities/HIprofile.h>
#include <iostream>
#include <sstream>
#include <vector>

namespace askap {

    namespace simulations {

      class HIprofileS3SAX : public HIprofile {
      public:
	HIprofileS3SAX(){};
	HIprofileS3SAX(std::string &line);
	virtual ~HIprofileS3SAX(){};
	HIprofileS3SAX(const HIprofileS3SAX& h);
	HIprofileS3SAX& operator= (const HIprofileS3SAX& h);

	void define();

	double flux(double nu);
	double flux(double nu1, double nu2);

	friend std::ostream& operator<< ( std::ostream& theStream, HIprofileS3SAX &prof);

      private:
	double itsFluxPeak;
	double itsFlux0;
	double itsWidthPeak;
	double itsWidth50;
	double itsWidth20;

	double itsIntFlux;
	
	double itsSideFlux;
	double itsMiddleFlux;

	std::vector<double> itsKpar;
	
      };

    }

}

#endif
