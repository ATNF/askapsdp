/// @file
/// 
/// @brief Calibration component (or individual effect).
/// @details The easiest way of creating individual components is
/// by deriving from this class. This class is mainly a structural
/// unit, but it holds the reference to parameters, which are passed
/// around to all components of the measurement equation.
///
/// @copyright (c) 2007 CONRAD, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef ME_COMPONENT_H
#define ME_COMPONENT_H

// own includes
#include <fitting/Params.h>

namespace conrad {

namespace synthesis {

/// @brief Calibration components (or individual effects).
/// @details The easiest way of creating individual components is
/// by deriving from this class. This class is mainly a structural
/// unit, but it holds the reference to parameters, which are passed
/// around to all components of the measurement equation.
/// @ingroup measurementequation
struct MEComponent {
   
   /// @brief constructor, store reference to paramters
   /// @param[in] par const reference to parameters
   inline explicit MEComponent(const scimath::Params &par) : itsParameters(par) {}
   
protected:
   /// @return reference to parameters
   inline const scimath::Params & parameters() const { return itsParameters; }
private:
   /// @brief reference to paramters
   const scimath::Params &itsParameters;
};

} // namespace synthesis

} // namespace conrad


#endif // #define ME_COMPONENT_H