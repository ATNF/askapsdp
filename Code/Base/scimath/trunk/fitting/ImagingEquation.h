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
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef IMAGING_EQUATION_H
#define IMAGING_EQUATION_H

#include <fitting/Equation.h>
#include <fitting/INormalEquations.h>
#include <fitting/ImagingNormalEquations.h>

namespace askap {

namespace scimath {

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
struct ImagingEquation : virtual public Equation {
    /// @brief default constructor
    ImagingEquation();
    
    /// @brief construct from specified parameters
    /// @param[in] ip parameters
    explicit ImagingEquation(const Params &ip);
    
    /// @brief calculate normal equations in the form specific to imaging 
    /// @details This method replaces calcEquations in the base class
    /// for a subclass of normal equations with imaging-specific approximation.
    /// This class implements calcEquations via this method checking whether
    /// the type of normal equations class supplied is compatible.
    /// @note Input type will be changed to ImagingNormalEquation, when
    /// this class is split out from the current NormalEquation
    /// @param[in] ne normal equations to update
    virtual void calcImagingEquations(ImagingNormalEquations &ne) const = 0;
    
    /// @brief calculate normal equations
    /// @details This is the main method defined in the base class which can
    /// accept any normal equations class. Concrete classes must check whether
    /// the type of the normal equations class compatible. An implementation
    /// of this method in this class does this check and executes 
    /// calcImagingEquations, if the type is appropriate. Override that method 
    /// in derived classes.
    /// @param[in] ne normal equations to update
    virtual void calcEquations(INormalEquations &ne) const;
};

} // namespace scimath

} // namespace askap

#endif // #ifndef IMAGING_EQUATION_H

