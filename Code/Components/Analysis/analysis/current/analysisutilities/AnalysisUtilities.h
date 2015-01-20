/// @file
///
/// Provides general utility functions to support the analysis code
///
/// @copyright (c) 2007 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#ifndef ASKAP_ANALYSIS_ANALYSISUTILS_H_
#define ASKAP_ANALYSIS_ANALYSISUTILS_H_

#include <analysisparallel/SubimageDef.h>
#include <askapparallel/AskapParallel.h>
#include <casainterface/CasaInterface.h>

#include <string>
#include <vector>

#include <Common/ParameterSet.h>
#include <casa/namespace.h>
#include <casa/Arrays/Array.h>
#include <scimath/Functionals/Gaussian2D.h>

#include <duchamp/Utils/Section.hh>
#include <duchamp/param.hh>
#include <duchamp/FitsIO/Beam.hh>
#include <duchamp/Detection/detection.hh>

namespace askap {
namespace analysis {

/// @ingroup analysisutilities
/// @{

/// @brief Return an array of axis dimensions for a FITS file.
std::vector<size_t> getFITSdimensions(std::string filename);

/// @brief Check for the use of a particular parameter in a
/// ParameterSet and warn the user it is not used.
void checkUnusedParameter(const LOFAR::ParameterSet& parset, std::string &paramName);

/// @brief Parse a ParameterSet and define duchamp::param parameters.
/// Takes a ParameterSet and reads in the necessary Duchamp
/// parameters. It checks many of the duchamp::param parameters, and
/// if they are not present, a default value, defined herein, is set
/// (note that this is not necessarily the standard Duchamp default
/// value).
duchamp::Param parseParset(const LOFAR::ParameterSet& parset);

std::string objectToSubsection(duchamp::Detection *object, long padding,
                               std::string imageName, duchamp::FitsHeader &header);

/// @}

}
}

#endif
