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
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef GENERIC_EQUATION_H
#define GENERIC_EQUATION_H

#include <fitting/Equation.h>
#include <fitting/INormalEquations.h>
#include <fitting/GenericNormalEquations.h>

namespace conrad {

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
struct GenericEquation : public Equation {
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
    virtual void calcEquations(INormalEquations &ne);
};

} // namespace scimath

} // namespace conrad

#endif // #ifndef GENERIC_EQUATION_H

