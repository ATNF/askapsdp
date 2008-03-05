/// @file
/// 
/// @brief Calibration component (or individual effect).
/// @details This class is mainly a structural unit. A derived class
/// ParameterizedMEComponent holds a reference to parameters.
///
/// @copyright (c) 2007 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#ifndef ME_COMPONENT_H
#define ME_COMPONENT_H

// own includes
#include <fitting/Params.h>

namespace askap {

namespace synthesis {

/// @brief Calibration component (or individual effect).
/// @details This class is mainly a structural unit. A derived class
/// ParameterizedMEComponent holds a reference to parameters.
/// @ingroup measurementequation
struct MEComponent {
   
};

} // namespace synthesis

} // namespace askap


#endif // #define ME_COMPONENT_H
