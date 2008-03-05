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
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#include <fitting/GenericEquation.h>
#include <askap/AskapError.h>

#include <stdexcept>

using namespace askap;
using namespace askap::scimath;

/// @brief default constructor
GenericEquation::GenericEquation() {}
    
/// @brief construct from specified parameters
/// @param[in] ip parameters
GenericEquation::GenericEquation(const Params &ip) : Equation(ip) {}

/// @brief calculate normal equations
/// @details This is the main method defined in the base class which can
/// accept any normal equations class. Concrete classes must check whether
/// the type of the normal equations class compatible. An implementation
/// of this method in this class does this check and executes 
/// calcGenericEquations if the type is appropriate. Override that method 
/// in the derived classes.
/// @param[in] ne normal equations to update
void GenericEquation::calcEquations(INormalEquations &ne) const
{
  try {
     calcGenericEquations(dynamic_cast<GenericNormalEquations&>(ne));
  }
  catch (const std::bad_cast &bc) {
     ASKAPTHROW(AskapError, "An attempt to use incompatible type of "
                 "the normal equations class with a derivative from "
                 "GenericEquation. It accepts only GenericNormalEquations "
                 "and derivatives. It probably indicates a logic error");    
  }
}
