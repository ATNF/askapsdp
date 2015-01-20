/// @file
///
/// XXX Notes on program XXX
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
#ifndef ASKAP_ANALYSIS_CATPREP_H_
#define ASKAP_ANALYSIS_CATPREP_H_

#include <sourcefitting/RadioSource.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/Outputs/CatalogueSpecification.hh>

#include <string>
#include <vector>

namespace askap {

namespace analysis {

/// Find the correct component suffix. Returns a string to
/// uniquely identify a fit that is part of an island. The first
/// 26 numbers (zero-based), get a single letter a-z. After that,
/// it becomes aa,ab,ac,...az,ba,bb,bc,...bz,ca,... If there are
/// more than 702 (=26^2+26), we move to three characters:
/// zy,zz,aaa,aab,aac,... And so on.
std::string getSuffix(unsigned int num);

/// Extract results from a 2D Gaussian suitable for printing to a
/// catalogue.This takes a 2D Guassian component, plus a
/// FitsHeader description and channel value, and returns the
/// deconvolved shape (as a vector), the ra & dec, and the
/// integrated flux. The FitsHeader is needed for the beam
/// correction and the WCS transform.
void getResultsParams(casa::Gaussian2D<Double> &gauss,
                      duchamp::FitsHeader *head, double zval,
                      std::vector<Double> &deconvShape, double &ra,
                      double &dec, double &intFluxFit);

/// Define an island catalogue spec based on the Duchamp catalogue
/// specification. This fills out all columns needed for the
/// island catalogue required by CASDA. The FitsHeader is needed
/// to get the flux and spectral units correct.
duchamp::Catalogues::CatalogueSpecification
IslandCatalogue(duchamp::FitsHeader &header);

/// Define a component catalogue specification conforming to CASDA
/// requirements. This fills out all columns required by
/// CASDA. The FitsHeader is needed to get the flux and spectral
/// units correct.
duchamp::Catalogues::CatalogueSpecification
ComponentCatalogue(duchamp::FitsHeader &header);

/// Define a component catalogue specification. This specification
/// is the "standard" Selavy one (not that required by
/// CASDA). This uses some columns defined by the Duchamp routines
/// (inputSpec), and usese the FitsHeader to get the units
/// correct.
duchamp::Catalogues::CatalogueSpecification
fullCatalogue(duchamp::Catalogues::CatalogueSpecification inputSpec,
              duchamp::FitsHeader &header);

/// Set up the columns for a component catalogue according to the
/// contents, ensuring the columns are wide enough for the values
/// therein. This can be used for either type of component
/// catalogue (ie. the CASDA version or the standard Selavy
/// version).
void setupCols(duchamp::Catalogues::CatalogueSpecification &spec,
               std::vector<sourcefitting::RadioSource> &srclist,
               std::string fitType);

}

}


#endif
