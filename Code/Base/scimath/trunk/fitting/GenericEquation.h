/// @file
/// @brief Measurement equation without any approximation
/// @details There are two kinds of normal equations currently supported. The
/// first one is a generic case, where the full normal matrix is retained. It
/// is used, e.g. for calibration. The second one is intended for imaging, where 
/// we can't afford to keep the whole normal matrix. In the latter approach, the 
/// matrix is approximated by a sum of diagonal and shift invariant matrices. 
/// This class represents a measurement equation in the general case, where no 
/// approximation to the normal matrix is done. It uses GenericNormalEquation as
/// opposed to ImagingNormalEquation
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

#ifndef GENERIC_EQUATION_H
#define GENERIC_EQUATION_H

#include <fitting/Equation.h>
#include <fitting/INormalEquations.h>
#include <fitting/GenericNormalEquations.h>

namespace askap {

namespace scimath {

/// @brief Measurement equation without any approximation
/// @details There are two kinds of normal equations currently supported. The
/// first one is a generic case, where the full normal matrix is retained. It
/// is used, e.g. for calibration. The second one is intended for imaging, where 
/// we can't afford to keep the whole normal matrix. In the latter approach, the 
/// matrix is approximated by a sum of diagonal and shift invariant matrices. 
/// This class represents a measurement equation in the general case, where no 
/// approximation to the normal matrix is done. It uses GenericNormalEquation as
/// opposed to ImagingNormalEquation
struct GenericEquation : virtual public Equation {
    /// @brief default constructor
    GenericEquation();
    
    /// @brief construct from specified parameters
    /// @param[in] ip parameters
    explicit GenericEquation(const Params &ip);
    
    /// @brief calculate normal equations in the general form 
    /// @details This method replaces calcEquations in the base class
    /// for a subclass of normal equations which don't do any approximation.
    /// This class implements calcEquations via this method checking whether
    /// the type of normal equations class supplied is compatible.
    /// @param[in] ne normal equations to update
    virtual void calcGenericEquations(GenericNormalEquations &ne) const = 0;
    
    /// @brief calculate normal equations
    /// @details This is the main method defined in the base class which can
    /// accept any normal equations class. Concrete classes must check whether
    /// the type of the normal equations class compatible. An implementation
    /// of this method in this class does this check and executes 
    /// calcGenericEquations if the type is appropriate. Override that method 
    /// in the derived classes.
    /// @param[in] ne normal equations to update
    virtual void calcEquations(INormalEquations &ne) const;
};

} // namespace scimath

} // namespace askap

#endif // #ifndef GENERIC_EQUATION_H

