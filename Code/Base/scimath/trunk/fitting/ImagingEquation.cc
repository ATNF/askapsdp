/// @file
/// @brief Measurement equation with an approximation used for imaging
/// @details There are two kinds of normal equations currently supported. The
/// first one is a generic case, where the full normal matrix is retained. It
/// is used, e.g. for calibration. The second one is intended for imaging, where 
/// we can't afford to keep the whole normal matrix. In the latter approach, the 
/// matrix is approximated by a sum of diagonal and shift invariant matrices. 
/// This class represents a measurement equation in the latter case with 
/// approximation. It simply serves as a structural element in the class
/// diagram and converts a call to generic calcEquation into a specific call
/// to fill a normal equation class appropriate for imaging.
/// @note Some imaging equations use generic normal equations. 
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <fitting/ImagingEquation.h>
#include <askap/AskapError.h>

#include <stdexcept>

using namespace askap;
using namespace askap::scimath;

/// @brief default constructor
ImagingEquation::ImagingEquation() {}
    
/// @brief construct from specified parameters
/// @param[in] ip parameters
ImagingEquation::ImagingEquation(const Params &ip) : Equation(ip) {}

/// @brief calculate normal equations
/// @details This is the main method defined in the base class which can
/// accept any normal equations class. Concrete classes must check whether
/// the type of the normal equations class compatible. An implementation
/// of this method in this class does this check and executes 
/// calcImagingEquations, if the type is appropriate. Override that method 
/// in derived classes.
/// @param[in] ne normal equations to update
void ImagingEquation::calcEquations(INormalEquations &ne) const
{
  try {
     calcImagingEquations(dynamic_cast<ImagingNormalEquations&>(ne));
  }
  catch (const std::bad_cast &bc) {
     ASKAPTHROW(AskapError, "An attempt to use incompatible type of "
                 "the normal equations class with a derivative from "
                 "ImagingEquation. It accepts only NormalEquations "
                 "and derivatives. It probably indicates a logic error");    
  }
}
